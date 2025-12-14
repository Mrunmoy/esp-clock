FROM espressif/idf:release-v5.5

# Set working directory
WORKDIR /project

# Copy project files
COPY . .

# Initialize and update git submodules
RUN git submodule update --init --recursive

# Set environment variables
ENV IDF_PATH=/project/third_party/esp-idf
ENV IDF_TOOLS_PATH=/project/third_party/esp-idf-tools

# Install ESP-IDF tools
RUN cd ${IDF_PATH} && ./install.sh esp32s3

# Default command - build the project
CMD ["/bin/bash", "-c", "source ${IDF_PATH}/export.sh && idf.py build"]
