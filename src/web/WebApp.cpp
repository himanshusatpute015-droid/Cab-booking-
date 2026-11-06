#include "web/WebApp.h"

#include <iostream>

#include "core/Utils.h"
#include "models/Payment.h"
#include "services/VehicleFactory.h"

namespace web {

using cab::Booking;
using cab::BookingStatus;
using cab::utils::fixed;
using cab::utils::toInt;
using cab::utils::toString;

WebApp::WebApp(cab::CabSystem& system, const TemplateEngine& views) : system(system), views(views) {}

// POINTER TO MEMBER FUNCTION + LAMBDA: wraps every handler so domain exceptions are
// turned into a friendly error page instead of crashing the request.
HttpServer::Handler WebApp::guarded(HttpResponse (WebApp::*action)(const HttpRequest&) const) const {
    const WebApp* self = this;
    return [self, action](const HttpRequest& request) -> HttpResponse {
        try {
            return (self->*action)(request);
        } catch (const cab::CabException& error) {  // catches every subclass polymorphically
            std::string back = request.header("Referer");
            return self->errorPage(error, back.empty() ? "/" : back);
        }
    };
}

void WebApp::registerRoutes(HttpServer& server) const {
    server.get("/", guarded(&WebApp::home));
    server.post("/estimate", guarded(&WebApp::estimate));
    server.post("/book", guarded(&WebApp::book));
    server.get("/booking", guarded(&WebApp::bookingDetails));
    server.get("/bookings", guarded(&WebApp::bookingList));
    server.post("/cancel", guarded(&WebApp::cancel));
    server.post("/complete", guarded(&WebApp::complete));
    server.get("/drivers", guarded(&WebApp::driverList));
    server.post("/drivers/add", guarded(&WebApp::addDriver));
    server.post("/drivers/toggle", guarded(&WebApp::toggleDriver));
    server.get("/dashboard", guarded(&WebApp::dashboard));
    server.get("/404", guarded(&WebApp::notFound));
}

// ===================================================================== helpers
HttpResponse WebApp::page(const std::string& title, const std::string& nav, const std::string& content,
                          int status) const {
    Context ctx;
    ctx.set("title", title).set("content", content);
    ctx.set("nav_home", nav == "home" ? "active" : "")
        .set("nav_bookings", nav == "bookings" ? "active" : "")
        .set("nav_drivers", nav == "drivers" ? "active" : "")
        .set("nav_dashboard", nav == "dashboard" ? "active" : "");
    return HttpResponse::html(views.render("layout.html", ctx), status);
}

HttpResponse WebApp::errorPage(const cab::CabException& error, const std::string& backLink) const {
    Context ctx;
    ctx.set("category", error.category()).set("message", error.what()).set("back", backLink);
    return page(error.category(), "", views.render("error.html", ctx), error.httpStatus());
}

std::string WebApp::locationOptions(const std::string& selected) const {
    std::string html;
    for (const cab::Location& loc : system.getLocations()) {
        Context ctx;
        ctx.set("value", loc.getName()).set("label", loc.getName()).set("selected", loc.getName() == selected ? "selected" : "");
        html += views.render("partials/option.html", ctx);
    }
    return html;
}

std::string WebApp::vehicleTypeOptions() const {
    std::string html;
    for (const auto& vehicle : cab::VehicleFactory::catalogue()) {
        Context ctx;
        ctx.set("value", vehicle->getType()).set("label", vehicle->icon() + " " + vehicle->getType()).set("selected", "");
        html += views.render("partials/option.html", ctx);
    }
    return html;
}

std::string WebApp::emptyRow(int columns, const std::string& message) const {
    Context ctx;
    ctx.set("cols", columns).set("message", message);
    return views.render("partials/empty_row.html", ctx);
}

std::string WebApp::statusClass(BookingStatus status) {
    switch (status) {
        case BookingStatus::Confirmed: return "badge-blue";
        case BookingStatus::Completed: return "badge-green";
        case BookingStatus::Cancelled: return "badge-red";
    }
    return "badge-gray";
}

std::string WebApp::bookingRows(const std::vector<std::shared_ptr<Booking>>& list, const std::string& emptyText) const {
    if (list.empty()) return emptyRow(8, emptyText);
    std::string html;
    for (const auto& b : list) {
        auto customer = system.getCustomer(b->getCustomerId());
        auto driver = system.getDriver(b->getDriverId());
        Context ctx;
        ctx.set("id", b->getId())
            .set("customer", customer->getName())
            .set("pickup", b->getPickup().getName())
            .set("drop", b->getDrop().getName())
            .set("icon", driver->getVehicle()->icon())
            .set("type", b->getVehicleType())
            .set("driver", driver->getName())
            .set("total", b->getFare().total())
            .set("status", Booking::statusToString(b->getStatus()))
            .set("status_class", statusClass(b->getStatus()))
            .set("created", b->getCreatedAt());
        html += views.render("partials/booking_row.html", ctx);
    }
    return html;
}

// ===================================================================== pages
HttpResponse WebApp::home(const HttpRequest&) const {
    cab::SystemStats stats = system.stats();

    std::string cards;
    for (const auto& vehicle : cab::VehicleFactory::catalogue()) {  // polymorphic loop
        Context c;
        c.set("icon", vehicle->icon())
            .set("type", vehicle->getType())
            .set("description", vehicle->description())
            .set("capacity", vehicle->getCapacity())
            .set("base", fixed(vehicle->baseFare(), 0))
            .set("per_km", fixed(vehicle->perKmRate(), 0));
        cards += views.render("partials/cab_card.html", c);
    }

    std::string coupons;
    for (const auto& coupon : system.getCoupons()) {
        Context c;
        c.set("code", coupon->getCode()).set("description", coupon->getDescription());
        coupons += views.render("partials/coupon_card.html", c);
    }

    Context ctx;
    ctx.set("pickup_options", locationOptions(""))
        .set("drop_options", locationOptions(""))
        .set("drivers_online", stats.availableDrivers)
        .set("total_rides", stats.totalBookings)
        .set("surge", fixed(stats.surge, 1))
        .set("cab_cards", cards)
        .set("coupon_cards", coupons);
    return page("Book a ride", "home", views.render("home.html", ctx));
}

HttpResponse WebApp::estimate(const HttpRequest& request) const {
    std::string pickup = request.param("pickup");
    std::string drop = request.param("drop");
    std::vector<cab::FareEstimate> estimates = system.estimateFares(pickup, drop);
    double km = system.distanceBetween(pickup, drop);

    std::string rows;
    bool checked = false;
    for (const cab::FareEstimate& e : estimates) {
        bool available = e.availableDrivers > 0;
        Context c;
        c.set("type", e.vehicle->getType())
            .set("icon", e.vehicle->icon())
            .set("capacity", e.vehicle->getCapacity())
            .set("availability", available ? toString(e.availableDrivers) + " nearby - " + toString(e.etaMinutes) + " min away"
                                           : std::string("No drivers available"))
            .set("base", e.fare.getBaseFare())
            .set("distance_fare", e.fare.getDistanceFare())
            .set("surge_fare", e.fare.getSurgeCharge())
            .set("extra", e.fare.getExtraCharge())
            .set("tax", e.fare.getTax())
            .set("total", e.fare.total())
            .set("state_class", available ? "" : "unavailable")
            .set("disabled", available ? "" : "disabled")
            .set("checked", available && !checked ? "checked" : "");
        if (available) checked = true;
        rows += views.render("partials/estimate_row.html", c);
    }

    std::string payments;
    for (const std::string& method : cab::PaymentFactory::methods()) {
        Context c;
        c.set("value", method).set("label", cab::PaymentFactory::create(method)->label()).set("selected", "");
        payments += views.render("partials/option.html", c);
    }

    Context ctx;
    ctx.set("pickup", pickup)
        .set("drop", drop)
        .set("distance", fixed(km, 1))
        .set("surge", fixed(system.currentSurge(), 1))
        .set("rows", rows)
        .set("payment_options", payments);
    return page("Choose your ride", "home", views.render("estimate.html", ctx));
}

HttpResponse WebApp::book(const HttpRequest& request) const {
    cab::BookingRequest r;
    r.riderName = request.param("name");
    r.riderPhone = request.param("phone");
    r.pickup = request.param("pickup");
    r.drop = request.param("drop");
    r.vehicleType = request.param("type");
    r.paymentMethod = request.param("payment");
    r.promoCode = request.param("promo");
    auto booking = system.bookRide(r);
    return HttpResponse::redirect("/booking?id=" + toString(booking->getId()));  // Post/Redirect/Get
}

HttpResponse WebApp::bookingDetails(const HttpRequest& request) const {
    auto b = system.getBooking(toInt(request.param("id"), -1));
    auto driver = system.getDriver(b->getDriverId());
    auto customer = system.getCustomer(b->getCustomerId());
    auto vehicle = driver->getVehicle();
    const cab::Fare& fare = b->getFare();

    Context actions;
    actions.set("id", b->getId());
    std::string actionsHtml;
    switch (b->getStatus()) {
        case BookingStatus::Confirmed: {
            auto payment = cab::PaymentFactory::create(b->getPaymentMethod());
            double fee = payment->processingFee(fare.total());
            actions.set("payable", fare.total() + fee).set("fee", fee).set("payment", payment->method());
            actionsHtml = views.render("partials/actions_confirmed.html", actions);
            break;
        }
        case BookingStatus::Completed:
            actions.set("stars", cab::utils::stars(b->getRating()))
                .set("paid", b->amountPaid())
                .set("fee", b->getPaymentFee())
                .set("payment", b->getPaymentMethod())
                .set("txn", b->getTransactionId());
            actionsHtml = views.render("partials/actions_completed.html", actions);
            break;
        case BookingStatus::Cancelled:
            actionsHtml = views.render("partials/actions_cancelled.html", actions);
            break;
    }

    Context ctx;
    ctx.set("id", b->getId())
        .set("created", b->getCreatedAt())
        .set("status", Booking::statusToString(b->getStatus()))
        .set("status_class", statusClass(b->getStatus()))
        .set("pickup", b->getPickup().getName())
        .set("drop", b->getDrop().getName())
        .set("distance", fixed(b->getDistanceKm(), 1))
        .set("base", fare.getBaseFare())
        .set("distance_fare", fare.getDistanceFare())
        .set("surge_fare", fare.getSurgeCharge())
        .set("extra", fare.getExtraCharge())
        .set("tax", fare.getTax())
        .set("discount", fare.getDiscount())
        .set("promo", b->getPromoCode().empty() ? "No promo applied" : "Promo " + b->getPromoCode())
        .set("total", fare.total())
        .set("payment", b->getPaymentMethod())
        .set("driver_initials", driver->initials())
        .set("driver_name", driver->getName())
        .set("driver_phone", driver->getPhone())
        .set("driver_rating", driver->ratingSummary())
        .set("vehicle_icon", vehicle->icon())
        .set("vehicle_type", vehicle->getType())
        .set("vehicle_model", vehicle->getModel())
        .set("vehicle_reg", vehicle->getRegistrationNo())
        .set("eta", b->getEtaMinutes())
        .set("customer_name", customer->getName())
        .set("customer_phone", customer->getPhone())
        .set("customer_tier", customer->tier())
        .set("actions", actionsHtml);
    return page("Booking #" + toString(b->getId()), "bookings", views.render("booking.html", ctx));
}

HttpResponse WebApp::bookingList(const HttpRequest& request) const {
    std::string phone = cab::utils::trim(request.param("phone"));
    std::vector<std::shared_ptr<Booking>> list = phone.empty() ? system.getBookings() : system.getBookingsByPhone(phone);
    std::string heading = phone.empty() ? "All bookings" : "Bookings for " + phone;
    std::string empty = phone.empty() ? "No rides booked yet. Book your first ride!" : "No bookings found for this phone number.";

    Context ctx;
    ctx.set("rows", bookingRows(list, empty))
        .set("phone", phone)
        .set("heading", heading)
        .set("count", static_cast<int>(list.size()));
    return page("Bookings", "bookings", views.render("bookings.html", ctx));
}

HttpResponse WebApp::cancel(const HttpRequest& request) const {
    int id = toInt(request.param("id"), -1);
    system.cancelBooking(id);
    return HttpResponse::redirect("/booking?id=" + toString(id));
}

HttpResponse WebApp::complete(const HttpRequest& request) const {
    int id = toInt(request.param("id"), -1);
    system.completeBooking(id, toInt(request.param("rating"), 0));
    return HttpResponse::redirect("/booking?id=" + toString(id));
}

HttpResponse WebApp::driverList(const HttpRequest&) const {
    std::string rows;
    for (const auto& d : system.getDrivers()) {
        auto v = d->getVehicle();
        std::string statusCss = !d->isOnline() ? "badge-gray" : (d->isOnRide() ? "badge-orange" : "badge-green");
        Context c;
        c.set("id", d->getId())
            .set("name", d->getName())
            .set("license", d->getLicenseNo())
            .set("phone", d->getPhone())
            .set("icon", v->icon())
            .set("type", v->getType())
            .set("model", v->getModel())
            .set("reg", v->getRegistrationNo())
            .set("location", d->getLocation().getName())
            .set("rating", d->ratingSummary())
            .set("rides", d->getCompletedRides())
            .set("earnings", d->getEarnings())
            .set("status", d->statusLabel())
            .set("status_class", statusCss)
            .set("toggle_label", d->isOnline() ? "Go offline" : "Go online")
            .set("toggle_disabled", d->isOnRide() ? "disabled" : "");
        rows += views.render("partials/driver_row.html", c);
    }
    if (rows.empty()) rows = emptyRow(9, "No drivers registered.");

    Context ctx;
    ctx.set("rows", rows)
        .set("count", static_cast<int>(system.getDrivers().size()))
        .set("type_options", vehicleTypeOptions())
        .set("location_options", locationOptions(""));
    return page("Drivers", "drivers", views.render("drivers.html", ctx));
}

HttpResponse WebApp::addDriver(const HttpRequest& request) const {
    system.addDriver(request.param("name"), request.param("phone"), request.param("license"), request.param("type"),
                     request.param("model"), request.param("reg"), request.param("location"));
    return HttpResponse::redirect("/drivers");
}

HttpResponse WebApp::toggleDriver(const HttpRequest& request) const {
    system.toggleDriver(toInt(request.param("id"), -1));
    return HttpResponse::redirect("/drivers");
}

HttpResponse WebApp::dashboard(const HttpRequest&) const {
    cab::SystemStats s = system.stats();

    std::string typeRows;
    for (const auto& entry : system.reportByVehicleType()) {
        Context c;
        c.set("type", entry.first).set("rides", entry.second.rides).set("revenue", entry.second.revenue);
        typeRows += views.render("partials/type_row.html", c);
    }

    std::string customerRows;
    for (const auto& customer : system.getCustomers()) {
        Context c;
        c.set("id", customer->getId())
            .set("name", customer->getName())
            .set("phone", customer->getPhone())
            .set("tier", customer->tier())
            .set("rides", customer->getTotalRides())
            .set("spent", customer->getTotalSpent());
        customerRows += views.render("partials/customer_row.html", c);
    }
    if (customerRows.empty()) customerRows = emptyRow(6, "No customers yet.");

    Context ctx;
    ctx.set("total_bookings", s.totalBookings)
        .set("active", s.activeRides)
        .set("completed", s.completedRides)
        .set("cancelled", s.cancelledRides)
        .set("drivers", s.totalDrivers)
        .set("available", s.availableDrivers)
        .set("customers", s.totalCustomers)
        .set("revenue", s.revenue)
        .set("payouts", s.driverPayouts)
        .set("commission", s.revenue - s.driverPayouts > 0 ? s.revenue - s.driverPayouts : 0.0)
        .set("avg_rating", fixed(s.averageRating, 2))
        .set("surge", fixed(s.surge, 1))
        .set("vehicles_in_memory", cab::Vehicle::getLiveCount())
        .set("type_rows", typeRows)
        .set("customer_rows", customerRows);
    return page("Dashboard", "dashboard", views.render("dashboard.html", ctx));
}

HttpResponse WebApp::notFound(const HttpRequest& request) const {
    cab::NotFoundException error("The page '" + request.getPath() + "' does not exist.");
    return errorPage(error, "/");
}

}  // namespace web
