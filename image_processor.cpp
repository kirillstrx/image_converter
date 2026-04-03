#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "parser.h"
#include "processor.h"

namespace {

void PrintHelp(const std::string& program_name) {
    std::cout
        << "Image Processor CLI\n"
        << "\n"
        << "Usage:\n"
        << "  " << program_name << " <input_file> <output_file> [filters]\n"
        << "\n"
        << "Examples:\n"
        << "  " << program_name << " input.bmp output.bmp\n"
        << "  " << program_name << " input.bmp output.bmp -gs\n"
        << "  " << program_name << " input.bmp output.bmp -crop 800 600 -gs\n"
        << "  " << program_name << " input.jpg output.png -blur 2.0 -contrast 1.2\n"
        << "  " << program_name << " input.heic output.jpeg -edge 0.2 -glow 2.0 0.35\n"
        << "\n"
        << "Available filters:\n"
        << "  -gs                    Convert image to grayscale\n"
        << "  -neg                   Invert image colors\n"
        << "  -sharp                 Apply sharpening\n"
        << "  -crop <width> <height> Crop image to the specified size\n"
        << "  -edge <threshold>      Edge detection after grayscale conversion\n"
        << "  -blur <sigma>          Gaussian blur\n"
        << "  -glow <sigma> <alpha>  Glow effect based on blur + blending\n"
        << "  -contrast <factor>     Adjust contrast\n"
        << "\n"
        << "Supported formats:\n"
        << "  Input:  BMP, PNG, JPEG/JPG, HEIC/HEIF\n"
        << "  Output: BMP, PNG, JPEG/JPG, HEIC/HEIF\n";
}

enum class ExitCode : int {
    kOk = 0,
    kInvalidArgs = 1,
    kProcessingError = 2,
    kUnknownError = 3,
};

}  // namespace

int main(const int argc, char** argv) {
    try {
        const std::string program_name = (argc > 0 && argv != nullptr && argv[0] != nullptr)
                                             ? argv[0]
                                             : "image_processor";

        if (argc == 1) {
            PrintHelp(program_name);
            return static_cast<int>(ExitCode::kOk);
        }

        std::vector<const char*> argv_view(argv, argv + argc);

        Parser parser;
        const ParsedCommand command = parser.Parse(argc, argv_view.data());

        Processor processor;
        processor.Run(command);

        std::cout << "Processing completed successfully\n";
        return static_cast<int>(ExitCode::kOk);
    } catch (const std::invalid_argument& error) {
        std::cerr << "Argument error: " << error.what() << "\n\n";
        const std::string program_name = (argc > 0 && argv != nullptr && argv[0] != nullptr)
                                             ? argv[0]
                                             : "image_processor";
        PrintHelp(program_name);
        return static_cast<int>(ExitCode::kInvalidArgs);
    } catch (const std::exception& error) {
        std::cerr << "Processing error: " << error.what() << '\n';
        return static_cast<int>(ExitCode::kProcessingError);
    } catch (...) {
        std::cerr << "Unknown error\n";
        return static_cast<int>(ExitCode::kUnknownError);
    }
}
