#ifndef CAB_WEB_TEMPLATE_ENGINE_H
#define CAB_WEB_TEMPLATE_ENGINE_H

#include <map>
#include <string>

namespace web {

// Holds the values that will be substituted into an HTML template.
// FUNCTION OVERLOADING: set() accepts strings, C-strings, ints and doubles.
// Each set() returns *this so calls can be chained (`ctx.set(a, 1).set(b, 2)`).
class Context {
private:
    std::map<std::string, std::string> values;

public:
    Context& set(const std::string& key, const std::string& value);
    Context& set(const std::string& key, const char* value);
    Context& set(const std::string& key, int value);
    Context& set(const std::string& key, double value);  // formatted as money (2 decimals)

    bool has(const std::string& key) const { return values.count(key) > 0; }
    std::string get(const std::string& key) const;
};

// Renders files from web/templates. Syntax (mustache-like):
//   {{name}}   -> value, HTML-escaped (safe for user input)
//   {{{name}}} -> value inserted raw (used for HTML fragments built from other templates)
class TemplateEngine {
private:
    std::string directory;
    std::string load(const std::string& name) const;

public:
    explicit TemplateEngine(const std::string& directory);
    std::string render(const std::string& name, const Context& context) const;
};

}  // namespace web

#endif
