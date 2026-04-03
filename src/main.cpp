#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "parser.h"
#include "processor.h"

enum class ExitCode : int {
    kOk = 0,
    kInvalidArgs = 1,
    kProcessingError = 2,
    kUnknownError = 3,
};

int main(const int argc, char** argv) {
    try {
        if (argc == 1) {
            const std::string program_name = argc > 0 ? argv[0] : "image_processor";
            return static_cast<int>(ExitCode::kOk);
        }

        std::vector<const char*> argv_view(argv, argv + argc);

        Parser parser;
        const ParsedCommand command = parser.Parse(argc, argv_view.data());

        Processor processor;
        processor.Run(command);

        return static_cast<int>(ExitCode::kOk);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Argument error: " << e.what() << std::endl;
        return static_cast<int>(ExitCode::kInvalidArgs);
    } catch (const std::exception& e) {
        std::cerr << "Processing error: " << e.what() << std::endl;
        return static_cast<int>(ExitCode::kProcessingError);
    } catch (...) {
        std::cerr << "Unknown error" << std::endl;
        return static_cast<int>(ExitCode::kUnknownError);
    }
}
