#include <glad/glad.h>       // GLAD must be included BEFORE GLFW — this is mandatory! 
#include <GLFW/glfw3.h>      // GLFW handles window creation and input 
#include <iostream>           // For std::cout debug output 

// ============================================= 
// VERTEX SHADER — runs on the GPU 
// Transforms each vertex position to screen space 
// ============================================= 
const char* vertexShaderSource = R"( 
    #version 330 core 
    layout (location = 0) in vec3 aPos;   // Vertex position (x, y, z) 
    layout (location = 1) in vec3 aColor; // Vertex colour (r, g, b) 
    out vec3 ourColor;                     // Pass colour to the fragment shader 
    void main() { 
        gl_Position = vec4(aPos, 1.0);     // Set the final clip-space position 
        ourColor = aColor;                 // Forward the colour unchanged 
    } 
)";

// ============================================= 
// FRAGMENT SHADER — determines the colour of each pixel 
// ============================================= 
const char* fragmentShaderSource = R"( 
    #version 330 core 
    in vec3 ourColor;      // Interpolated colour received from vertex shader 
    out vec4 FragColor;    // Final output colour (r, g, b, alpha) 
    void main() { 
        FragColor = vec4(ourColor, 1.0); // Alpha = 1.0 means fully opaque 
    } 
)";

// Helper function: compile a shader and report any errors 
unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    // Check for compilation errors 
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "SHADER COMPILE ERROR:\n" << infoLog << std::endl;
    }
    return shader;
}

int main() {
    // ==========================================
    // STEP 1: Initialise GLFW 
    // ========================================== 
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Request OpenGL 3.x 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Request OpenGL x.3 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Core profile only 

    // ========================================== 
    // STEP 2: Create the window (800x600) 
    // ========================================== 
    GLFWwindow* window = glfwCreateWindow(1024, 768, "NUR ATIQAH BT KAMARUZZAMAN", NULL,
        NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // Make this window the active OpenGL context 

    // ========================================== 
    // STEP 3: Initialise GLAD — loads all OpenGL function pointers 
    // ========================================== 
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialise GLAD." << std::endl;
        return -1;
    }

    // ========================================== 
    // STEP 4: Set the viewport (rendering area) 
    // ========================================== 
    glViewport(0, 0, 800, 600);

    // ========================================== 
    // STEP 5: Compile and link the shaders 
    // ========================================== 
    unsigned int vertShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Link both compiled shaders into a single shader program 
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertShader); // Individual shaders no longer needed after linking 
    glDeleteShader(fragShader);

    // ========================================== 
    // STEP 6: Define the triangle vertex data 
    // OpenGL coordinate range: -1.0 to +1.0 (NDC — Normalised Device Coordinates) 
    // Each vertex: x, y, z,  r, g, b 
    // ========================================== 
    float vertices[] = {
        // Triangle 1 (left side)
        -0.5f,  0.5f, 0.0f,    1.0f, 0.0f, 0.0f,  // Red
        -0.9f, -0.5f, 0.0f,    0.0f, 1.0f, 0.0f,  // Green
        -0.1f, -0.5f, 0.0f,    0.0f, 0.0f, 1.0f,  // Blue

        // Triangle 2 (right side)
         0.1f,  0.5f, 0.0f,    1.0f, 1.0f, 0.0f,  // Yellow
         0.5f, -0.5f, 0.0f,    1.0f, 0.0f, 1.0f,  // Purple
         0.9f,  0.5f, 0.0f,    0.0f, 1.0f, 1.0f   // Cyan
    };

    // ========================================== 
    // STEP 7: Upload vertex data to the GPU using VAO and VBO 
    // VAO (Vertex Array Object)  — remembers HOW the vertex data is laid out 
    // VBO (Vertex Buffer Object) — stores the actual vertex data on the GPU 
    // ========================================== 
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO); // Create 1 VAO 
    glGenBuffers(1, &VBO);      // Create 1 VBO 

    glBindVertexArray(VAO);              // Bind VAO first — it records all state below 
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  // Bind VBO 
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // GL_STATIC_DRAW: data will not change — GPU optimises accordingly 

    // Tell the GPU: attribute 0 = position (3 floats, stride 6 floats, offset 0) 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Tell the GPU: attribute 1 = colour (3 floats, stride 6 floats, offset 3 floats) 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 *
        sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); // Unbind the VAO (good practice) 

    // ========================================== 
    // STEP 8: Render loop — runs until the window is closed 
    // ========================================== 
    while (!glfwWindowShouldClose(window)) {
        // Press ESC to close the window 
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Clear the screen with a dark background 
        glClearColor(0.0f, 0.3f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Activate our shader program 
        glUseProgram(shaderProgram);

        // Draw the triangle (3 vertices) 
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window); // Swap front/back buffers (double buffering) 
        glfwPollEvents();        // Process keyboard, mouse and window events 
    }

    // ========================================== 
    // STEP 9: Cleanup — release all allocated resources 
    // ========================================== 
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}