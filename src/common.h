inline unsigned int hash(unsigned int a) {
    a = (a + 0x7ed55d16) + (a << 12);
    a = (a ^ 0xc761c23c) ^ (a >> 19);
    a = (a + 0x165667b1) + (a << 5);
    a = (a + 0xd3a2646c) ^ (a << 9);
    a = (a + 0xfd7046c5) + (a << 3);
    a = (a ^ 0xb55a4f09) ^ (a >> 16);
    return a;
}

inline void compileShader(GLuint program, const std::string &vertex_shader_src,
                          const std::string &fragment_shader_src) {
    // compile shader
    GLint length;
    const GLchar *source;

    const GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    length = vertex_shader_src.size();
    source = vertex_shader_src.c_str();
    glShaderSource(vert, 1, &source, &length);

    const GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    length = fragment_shader_src.size();
    source = fragment_shader_src.c_str();
    glShaderSource(frag, 1, &source, &length);

    glCompileShader(vert);
    glCompileShader(frag);

    GLint isCompiled = 0;
    glGetShaderiv(vert, GL_COMPILE_STATUS, &isCompiled);
    if (!isCompiled)
        goto error;
    glGetShaderiv(frag, GL_COMPILE_STATUS, &isCompiled);
    if (!isCompiled)
        goto error;

    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    glDetachShader(program, vert);
    glDetachShader(program, frag);
    GLint isLinked;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (!isLinked)
        goto error;

    glDeleteShader(vert);
    glDeleteShader(frag);

    return;
error:
    fprintf(stderr, "fail compiling OpenGL shader\n");
    exit(1);
}