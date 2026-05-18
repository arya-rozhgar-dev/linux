#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    int file_descriptor;
    char send_string[] = "Hello from the User-Space Application Layer!";
    char receive_buffer[1024] = {0};

    printf("--- Application Started ---\n");

    // 1. Open the custom kernel bridge
    file_descriptor = open("/dev/app_bridge", O_RDWR);
    if (file_descriptor < 0) {
        perror("FATAL: Failed to connect to the custom kernel driver. Is the module loaded?");
        return -1;
    }
    printf("[+] Successfully opened /dev/app_bridge\n");

    // 2. Write data straight into kernel memory
    printf("[+] Sending to Kernel: %s\n", send_string);
    if (write(file_descriptor, send_string, strlen(send_string)) < 0) {
        perror("Failed to write to kernel");
    }

    // 3. Read the data back from the kernel
    printf("[+] Reading response from Kernel...\n");
    if (read(file_descriptor, receive_buffer, sizeof(receive_buffer)) < 0) {
        perror("Failed to read from kernel");
    }
    printf("[+] Received: %s\n", receive_buffer);

    // 4. Close the bridge
    close(file_descriptor);
    printf("--- Application Terminated ---\n");
    
    return 0;
}
