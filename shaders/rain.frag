#version 410 core
out vec4 FragColor;

void main() {
    // Create a circular point sprite
    vec2 circCoord = 2.0 * gl_PointCoord - 1.0;
    float circle = dot(circCoord, circCoord);
    if (circle > 1.0) {
        discard;
    }
    
    // Light blue color for rain drops
    FragColor = vec4(0.7, 0.7, 1.0, 0.5);
}