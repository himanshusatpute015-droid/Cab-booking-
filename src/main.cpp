// CabGo - Cab Booking System
// All application logic is written in C++. HTML/CSS files in web/ are used only for
// presentation and are filled in by the C++ TemplateEngine.

#include <cstdlib>
#include <exception>
#include <iostream>

#include "core/Utils.h"
#include "services/CabSystem.h"
#include "web/HttpServer.h"
#include "web/TemplateEngine.h"
#include "web/WebApp.h"

int main(int argc, char* argv[]) {
    int port = argc > 1 ? cab::utils::toInt(argv[1], 8080) : 8080;

    try {
        cab::CabSystem& system = cab::CabSystem::instance();
        system.init("data");

        web::TemplateEngine views("web/templates");
        web::HttpServer server(port, "web/static");
        web::WebApp app(system, views);
        app.registerRoutes(server);

        std::cout << "\n  CabGo is running!  Open http://localhost:" << port << " in your browser.\n"
                  << "  Press Ctrl+C to stop.\n"
                  << std::endl;
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
