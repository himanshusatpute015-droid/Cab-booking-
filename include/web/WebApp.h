#ifndef CAB_WEB_WEB_APP_H
#define CAB_WEB_WEB_APP_H

#include <string>

#include "core/Exceptions.h"
#include "services/CabSystem.h"
#include "web/HttpServer.h"
#include "web/TemplateEngine.h"

namespace web {

// CONTROLLER: translates HTTP requests into CabSystem calls and renders the results
// with HTML templates. It contains no business rules itself.
class WebApp {
private:
    cab::CabSystem& system;          // association (reference, not ownership)
    const TemplateEngine& views;

    HttpServer::Handler guarded(HttpResponse (WebApp::*action)(const HttpRequest&) const) const;
    HttpResponse page(const std::string& title, const std::string& nav, const std::string& content,
                      int status = 200) const;
    HttpResponse errorPage(const cab::CabException& error, const std::string& backLink) const;

    std::string locationOptions(const std::string& selected) const;
    std::string vehicleTypeOptions() const;
    std::string bookingRows(const std::vector<std::shared_ptr<cab::Booking>>& list, const std::string& emptyText) const;
    std::string emptyRow(int columns, const std::string& message) const;
    static std::string statusClass(cab::BookingStatus status);

    // Route handlers
    HttpResponse home(const HttpRequest& request) const;
    HttpResponse estimate(const HttpRequest& request) const;
    HttpResponse book(const HttpRequest& request) const;
    HttpResponse bookingDetails(const HttpRequest& request) const;
    HttpResponse bookingList(const HttpRequest& request) const;
    HttpResponse cancel(const HttpRequest& request) const;
    HttpResponse complete(const HttpRequest& request) const;
    HttpResponse driverList(const HttpRequest& request) const;
    HttpResponse addDriver(const HttpRequest& request) const;
    HttpResponse toggleDriver(const HttpRequest& request) const;
    HttpResponse dashboard(const HttpRequest& request) const;
    HttpResponse notFound(const HttpRequest& request) const;

public:
    WebApp(cab::CabSystem& system, const TemplateEngine& views);
    void registerRoutes(HttpServer& server) const;
};

}  // namespace web

#endif
