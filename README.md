# 🚕 CabGo: a Cab Booking System in C++

CabGo is a working cab booking web app built to show **Object-Oriented Programming in C++**.

- **All of the logic is C++**: the web server, routing, fare calculation, driver matching, payments, promo codes, validation and file storage.
- **HTML and CSS are used only for the interface.** The pages contain no JavaScript. The C++ code fills in `{{placeholders}}` in the HTML templates and handles every form submission.
- **It needs no external libraries.** The HTTP server is written from scratch on top of OS sockets (WinSock on Windows, BSD sockets on Linux and macOS).

---

## ✨ Features

| Feature | Description |
|---|---|
| Fare estimates | Compares the price of all 6 cab types (Bike, Auto, Mini, Sedan, SUV, Premier) for any route |
| Smart driver matching | Assigns the **nearest available** driver of the chosen type and breaks ties by rating |
| Dynamic surge pricing | The surge multiplier (1.0x–1.8x) depends on how many online drivers are busy |
| Promo codes | `FIRST50`, `SAVE30`, `RIDE10`, `LOYAL20`, each with its own rules |
| Payments | UPI, Cash, Card (2% fee) and Wallet (1% fee), handled through a common interface |
| Ride lifecycle | Book → Complete (pay and rate the driver) or Cancel |
| Driver management | Register drivers and switch them online or offline; the app tracks earnings and ratings |
| Booking history | Lists all bookings, with search by phone number |
| Dashboard | Shows revenue, commission, revenue by cab type, customers and their tiers |
| Persistence | Everything is saved to text files in `data/` and reloaded at start-up |
| Validation and errors | Invalid input shows a friendly error page with the correct HTTP status code |

---

## 🧠 OOP Concepts Used (and where to find them)

| # | Concept | Where in the code |
|---|---|---|
| 1 | **Classes & Objects** | Every file in `include/models`, for example `Driver`, `Booking` and `Fare` |
| 2 | **Encapsulation** | Private data with getters and validating setters: `Person::setPhone()`, `Location`, `Booking` |
| 3 | **Abstraction** (abstract classes) | `Person`, `Vehicle`, `Car`, `Coupon` and `Payment` all have pure virtual functions |
| 4 | **Interface** (pure abstract class) | `Payment` declares only pure virtual methods; `Rateable` is a mixin |
| 5 | **Single inheritance** | `Customer : Person`, `Bike : Vehicle` |
| 6 | **Multiple inheritance** | `Driver : public Person, public Rateable` |
| 7 | **Multilevel inheritance** | `Vehicle → Car → Sedan → Premier` |
| 8 | **Hierarchical inheritance** | `Vehicle` has `Bike`, `AutoRickshaw` and `Car`; `Car` has `Mini`, `Sedan` and `SUV` |
| 9 | **Runtime polymorphism** (virtual functions) | `Vehicle::calculateFare()` calls the virtual `baseFare()`, `perKmRate()` and `extraCharges()`; `Coupon::discountFor()`; `Payment::pay()` |
| 10 | **Function overriding and calling the base version** | `Premier::extraCharges()` calls `Sedan::extraCharges()`; `Driver::profile()` calls `Person::profile()` |
| 11 | **Compile-time polymorphism: function overloading** | `Context::set()` takes a `string`, `const char*`, `int` or `double` |
| 12 | **Constructor overloading** | `Customer(name, phone)` and `Customer(id, name, phone, rides, spent)` |
| 13 | **Operator overloading** | `Location::operator-` returns the distance, `Fare::operator+` / `+=` / `<`, and `operator<<` for `Fare`, `Booking` and `Location` |
| 14 | **Constructors & destructors** | Default, parameterised, **copy** (`Location`, `Fare`) and **delegating** (`Driver`, `Customer`) constructors; `Vehicle::~Vehicle()` updates a counter |
| 15 | **Virtual destructors** | `Person`, `Vehicle`, `Payment` and `Coupon`, so objects are deleted safely through base pointers |
| 16 | **Static members** | `Booking::nextId`, `Vehicle::liveCount`, `Fare::GST_RATE`, `Driver::DRIVER_SHARE` and static factory functions |
| 17 | **Friend function** | `operator<<(ostream&, const Fare&)` reads private fields directly |
| 18 | **Friend class** | `FileStorage` is a friend of `Booking`, `Driver` and `Customer` so it can restore their private state |
| 19 | **`this` pointer** | Self-assignment checks in `operator=`; `Context::set()` returns `*this` so calls can be chained |
| 20 | **Templates (class)** | `Repository<T, Key>` is one generic store used for customers, drivers and bookings |
| 21 | **Templates (function)** | `utils::toString<T>()`, `Repository::filter<Predicate>()` |
| 22 | **Exception handling** | A custom hierarchy rooted at `CabException`: `ValidationException`, `NotFoundException`, `NoDriverAvailableException`, `InvalidStateException`, `PaymentException` |
| 23 | **Composition / Aggregation / Association** | A `Driver` *has a* `Vehicle` and a `Location`; `CabSystem` *aggregates* the repositories; a `Booking` *associates* a customer with a driver |
| 24 | **Smart pointers & RAII** | `shared_ptr` and `unique_ptr` everywhere; `HttpServer` closes its socket in its destructor; `SocketRuntime` starts and stops WinSock |
| 25 | **`final`, `override`, `= delete`** | `class Premier final`; `override` on every overridden function; `CabSystem` cannot be copied |
| 26 | **Namespaces** | `cab::` for the domain, `cab::utils::` for helpers, `web::` for the HTTP layer |
| 27 | **File handling** | `FileStorage` uses `ifstream` and `ofstream` to read and write `data/*.txt` |
| 28 | **Pointer to member function & lambdas** | `WebApp::guarded(&WebApp::home)` wraps each route handler |
| 29 | **Enum class** | `BookingStatus { Confirmed, Completed, Cancelled }` |

### Design patterns
- **Singleton**: `CabSystem::instance()`
- **Factory**: `VehicleFactory`, `PaymentFactory`
- **Strategy**: the `Payment` subclasses and the `Coupon` subclasses
- **Template Method**: `Vehicle::calculateFare()`
- **MVC**: models (`include/models`), controller (`WebApp`), views (`web/templates`)

---

## 🏗️ Architecture

```
 Browser (HTML + CSS only)
        │  HTTP GET / POST (plain HTML forms)
        ▼
 ┌─────────────────────────────── C++ ────────────────────────────────┐
 │ web::HttpServer   raw sockets, parses requests, routes to handlers │
 │        │                                                           │
 │ web::WebApp       controller: reads form fields, calls CabSystem,  │
 │        │          fills HTML templates with web::TemplateEngine    │
 │        ▼                                                           │
 │ cab::CabSystem    singleton facade holding all the business rules  │
 │   ├─ Repository<Customer> / Repository<Driver> / Repository<Booking>│
 │   ├─ VehicleFactory, PaymentFactory, Coupons                       │
 │   └─ FileStorage  ──►  data/customers.txt, drivers.txt, bookings.txt│
 └────────────────────────────────────────────────────────────────────┘
```

### Project structure
```
├── include/
│   ├── core/       Utils.h, Exceptions.h, Repository.h (template)
│   ├── models/     Person, Customer, Driver, Rateable, Vehicle, Location,
│   │               Fare, Booking, Payment, Coupon
│   ├── services/   CabSystem, VehicleFactory, FileStorage
│   └── web/        HttpServer, HttpRequest, HttpResponse, TemplateEngine, WebApp
├── src/            .cpp implementation files (same layout) + main.cpp
├── web/
│   ├── templates/  HTML pages + partials/ (fragments repeated by C++ loops)
│   └── static/     style.css
├── build.bat, run.bat   Windows build and run scripts
└── Makefile             Linux / macOS / MinGW build
```

---

## ▶️ Build & Run

**Requirement:** a C++14 compiler (g++ 6 or newer, or clang). Nothing else is needed.

### Windows (MinGW)
```bat
build.bat
cabgo.exe
```
You can also double-click **`run.bat`**, which builds the program, starts it and opens your browser.

To build by hand:
```bat
g++ -std=c++14 -O2 -Iinclude src\main.cpp src\core\*.cpp src\models\*.cpp src\services\*.cpp src\web\*.cpp -o cabgo.exe -lws2_32
```

### Linux / macOS
```bash
make
./cabgo
```

Then open **http://localhost:8080** in a browser. To use a different port, pass it as an argument: `cabgo.exe 9000`.

> Run the program from the project root folder so it can find `web/` and `data/`.
> To reset the app to its demo data, stop the server and delete the `data/` folder. Twelve demo drivers are created again on the next start.

---

## 🧭 How to use

1. **Book a Ride:** choose a pickup and a drop location, then click *See prices*.
2. **Compare fares:** every cab type shows its full fare breakdown and how far away the nearest driver is. Pick one, enter your name, phone number and payment method, and add an optional promo code (try `FIRST50` on your first ride).
3. **Booking page:** see your assigned driver, vehicle number plate, ETA and fare receipt. From here you can:
   - **Complete the ride**: rate the driver (1–5 ★) and pay; a transaction ID is generated.
   - **Cancel** the booking; the driver becomes available again.
4. **Bookings:** see the history of every ride, or search by phone number.
5. **Drivers:** register new drivers and switch them online or offline.
6. **Dashboard:** see revenue, commission and statistics by cab type and by customer.

### Fare formula
```
fare  = base + max(distance × per-km rate, minimum fare − base)
fare += surge charge (fare × (surge − 1))
fare += extras  (AC: ₹1/km · SUV: +₹30 · Premier: +₹50)
fare += GST 5%
fare −= promo discount
```
Distance is the straight-line distance between two city points multiplied by 1.3 to approximate road distance.

---

## 📄 License
MIT. Free to use for learning.
