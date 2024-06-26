    "precision mediump float;                                                     \n"
    "uniform vec3 u_camNormal;                   \n"
    "varying vec3 v_normal;                  \n"
    "varying vec3 v_color;                                                     \n"
    "varying vec3 v_lightToPointDir;                  \n"
    "varying mat3 rotSubmatrix;                  \n"
    "varying vec2 v_texelCoord;                  \n"
    "uniform sampler2D textureSample; \n"
    "void main()                                                                  \n"
    "{                                                                            \n"
    "  float cosAngle = dot(v_lightToPointDir, rotSubmatrix * v_normal);\n"
    "  vec4 final_color = vec4((cosAngle * v_color) + (0.6 * v_color), 1.0); \n"
    "  vec4 texelColor = texture2D(textureSample, v_texelCoord); \n"
    "  final_color = vec4((cosAngle * final_color) + (0.6 * texelColor)); \n"
    "  vec4 minColor = vec4(0.3 * final_color); \n"
    "  minColor.a = 1.0; \n"
    "  vec4 maxColor = vec4(1.0); \n"
    "  gl_FragColor = clamp(final_color, minColor, maxColor);        \n"
    "}                                                                            \n";


