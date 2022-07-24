# look for Doxygen package
# Require dot, treat the other components as optional
find_package(Doxygen
             REQUIRED dot
             OPTIONAL_COMPONENTS mscgen dia)

if(DOXYGEN_FOUND)
  # doxygen settings can be set here, prefixed with "DOXYGEN_"
  set(DOXYGEN_PROJECT_NAME "SpOCK")
  set(DOXYGEN_PROJECT_LOGO "spock.png")
  set(DOXYGEN_EXCLUDE_PATTERNS "README.md" "doxygen-awesome-css")
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "main-page.md")
  set(DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/doc")
  set(DOXYGEN_HTML_EXTRA_STYLESHEET "doxygen-awesome-css/doxygen-awesome.css" "doxygen-awesome-css/doxygen-awesome-sidebar-only.css")
  set(DOXYGEN_GENERATE_TREEVIEW YES) #require by doxygen awesome
  set(DOXYGEN_HTML_HEADER "header.html")
  set(DOXYGEN_USE_MATHJAX YES)
  # this target will only be built if specifically asked to.
  # run "make doc" to create the doxygen documentation
  doxygen_add_docs(
    doc
    ${PROJECT_SOURCE_DIR}
    COMMENT "Generate API-documents for NoteSearch."
  )
endif(DOXYGEN_FOUND)
