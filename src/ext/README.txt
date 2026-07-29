Bundled third-party source used by the current build:

- `mpegsound`: repaired and C++17-modernized Splay MPEG audio decoder sources;
  the bundled decoder is exercised by a full-track sanitizer regression test
- `imgui`: Dear ImGui and its GLFW/OpenGL 2 editor backends
- `stb`: image-resizing support used by texture mipmap generation
- `glh`: the small legacy OpenGL extension-loader subset still used by the
  compatibility renderer

The former GLUI toolkit and extension-generator utility are no longer part of
the supported source tree.
