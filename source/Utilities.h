//
// Created by Бушев Дмитрий on 01.07.2021.
//

#ifndef VULGINE_UTILITIES_H
#define VULGINE_UTILITIES_H
#include <string>
#include <ostream>
#include <vulkan/vulkan.h>

namespace Vulgine{

    namespace Utilities {
        class Logger {
            std::ostream *output;
            bool enabled = true;
        public:

            explicit Logger(std::ostream *stream) : output(stream) {}

            void operator()(std::string const &log);

            void changeLogFile(std::ostream *new_out) {
                output = new_out;
            }

            [[nodiscard]] bool hasNullLogFile() const {
                return output == nullptr;
            }

            void enable() {
                enabled = true;
            }

            void disable() {
                enabled = false;
            }
        };
    }

    extern Utilities::Logger logger;

    namespace Utilities{
        class Errs {
            std::string lastError;
        public:
            void operator()(std::string const &err) {
                logger("[ERROR]: " + err);
                lastError = err;
            }

            [[nodiscard]] std::string get() const {

                return lastError;
            }
        };

        std::string errorString(VkResult errorCode);
        void ExitFatal(int err_code = -1, std::string const& err = "program aborted");
    }
    extern Utilities::Errs errs;

}

#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		errs("Fatal : VkResult is \"" + ::Vulgine::Utilities::errorString(res) + "\" in " + __FILE__ + " at line " + std::to_string(__LINE__) + "\n"); \
		assert(res == VK_SUCCESS);																		\
	}																									\
}

#endif //VULGINE_UTILITIES_H
