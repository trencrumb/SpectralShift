# This file creates a .env file in the build directory
# that can be sourced by GitHub Actions to set environment variables

# Get properties from the target
get_target_property(PRODUCT_NAME ${PROJECT_NAME} JUCE_PRODUCT_NAME)
get_target_property(COMPANY_NAME ${PROJECT_NAME} JUCE_COMPANY_NAME)

# Write the .env file to the build directory
file(WRITE "${CMAKE_BINARY_DIR}/.env"
    "PRODUCT_NAME=${PRODUCT_NAME}\n"
    "PROJECT_NAME=${PROJECT_NAME}\n"
    "VERSION=${PROJECT_VERSION}\n"
    "COMPANY_NAME=${COMPANY_NAME}\n"
    "BUNDLE_ID=com.${COMPANY_NAME}.${PROJECT_NAME}\n"
)
message(STATUS "Created .env file at ${CMAKE_BINARY_DIR}/.env")
