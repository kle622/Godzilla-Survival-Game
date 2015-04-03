Final Project
Kevin Le

Made one change to the main.cpp file that has to do with my development environment,

used glad instead of glew, for some reason glew doesn't like my environment. I used
the same setup that was in the Project directory you emailed out earlier this quarter.

Specifically, I changed

    if (glewInit() != GLEW_OK) {
      fprintf(stderr, "Failed to initialize GLEW\n");
      return -1;
    }

    to

   if (!gladLoadGL()) {
      fprintf(stderr, "Unable to initialize glad");
      glfwDestroyWindow(window);
      glfwTerminate();
      return 1;
   }

Thanks!