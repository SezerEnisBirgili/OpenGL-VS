#pragma once

// Unique vertices: 4 vertices per face * 6 faces = 24 vertices total
// Format: Position (3) | Normal (3) | TexCoord (2)
inline float basicCubeVertices[] = {
    // Face 0 – back (-Z)
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, // 0: Bottom-Left
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f, // 1: Top-Left
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f, // 2: Top-Right
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f, // 3: Bottom-Right

    // Face 1 – front (+Z)
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f, // 4: Bottom-Left
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f, // 5: Bottom-Right
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f, // 6: Top-Right
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f, // 7: Top-Left

    // Face 2 – left (-X)
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f, // 8: Top-Right
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f, // 9: Top-Left
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f, // 10: Bottom-Left
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f, // 11: Bottom-Right

    // Face 3 – right (+X)
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f, // 12: Top-Left
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, // 13: Bottom-Right
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, // 14: Top-Right
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, // 15: Bottom-Left

     // Face 4 – Bottom (-Y)
     -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f, // 16: Bottom-Left
      0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f, // 17: Bottom-Right
      0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f, // 18: Top-Right
     -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f, // 19: Top-Left

     // Face 5 – top (+Y)
     -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f, // 20: Bottom-Left
     -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f, // 21: Top-Left
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f, // 22: Top-Right
      0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f  // 23: Bottom-Right
};

inline unsigned int cubeIndices[] = {
    // Face 0 – back (-Z)
    0, 1, 2,   2, 3, 0,

    // Face 1 – front (+Z)
    4, 5, 6,   6, 7, 4,

    // Face 2 – left (-X)
    8, 9, 10,  10, 11, 8,

    // Face 3 – right (+X)
    12, 13, 14, 13, 12, 15,

    // Face 4 – Bottom (-Y)
    16, 17, 18, 18, 19, 16,

    // Face 5 – top (+Y)
    20, 21, 22, 22, 23, 20
};

inline float basicCubeWrappedTextureVertices[] = {
    // Face 1 – front (+Z) -> AMERICAS
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.000f, 0.500f, // 0: Left Bottom
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.333f, 0.500f, // 1: Right Bottom
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.333f, 1.000f, // 2: Right Top
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.000f, 1.000f, // 3: Left Top

    // Face 0 – back (-Z) -> ASIA
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.666f, 0.500f, // 4: Right Bottom
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.666f, 1.000f, // 5: Right Top
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.333f, 1.000f, // 6: Left Top
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.333f, 0.500f, // 7: Left Bottom

    // Face 2 – left (-X) -> EUROPE AND AFRICA
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.000f, 1.000f, // 8: Right Top
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.666f, 1.000f, // 9: Left Top
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.666f, 0.500f, // 10: Left Bottom
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.000f, 0.500f, // 11: Right Bottom

    // Face 3 – right (+X) -> INDIA
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.000f, 0.500f, // 12: Left Top
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.333f, 0.000f, // 13: Right Bottom
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.333f, 0.500f, // 14: Right Top
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.000f, 0.000f, // 15: Left Bottom

     // Face 5 – top (+Y) -> NORTH POLE
     -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.333f, 0.000f, // 16: Left Bottom
     -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.333f, 0.500f, // 17: Left Top
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.666f, 0.500f, // 18: Right Top
      0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.666f, 0.000f, // 19: Right Bottom

     // Face 4 – Bottom (-Y) -> SOUTH POLE
     -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.666f, 0.000f, // 20: Left Bottom
      0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.000f, 0.000f, // 21: Right Bottom
      0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.000f, 0.500f, // 22: Right Top
     -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.666f, 0.500f  // 23: Left Top
};