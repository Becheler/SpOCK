# look for Doxygen package
# Require dot, treat the other components as optional
find_package(Doxygen
             REQUIRED dot
             OPTIONAL_COMPONENTS mscgen dia)

if(DOXYGEN_FOUND)
  # exclude sqlite code
  set(DOXYGEN_EXCLUDE_PATTERNS
        */sqlite3/*
  )
  # doxygen settings can be set here, prefixed with "DOXYGEN_"
  set(DOXYGEN_PROJECT_NAME "SpOCK")
  set(DOXYGEN_PROJECT_LOGO "spock.png")
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "mainpage.md")
  set(DOXYGEN_EXCLUDE_PATTERNS "README.md")
  set(DOXYGEN_EXTRACT_PRIVATE YES)
  set(DOXYGEN_INLINE_INHERITED_MEMB YES)
  set(DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/doc")
  set(DOXYGEN_HTML_EXTRA_STYLESHEET "customdoxygen.css")
  set(DOXYGEN_HTML_HEADER "header.html")
  set(DOXYGEN_HTML_FOOTER "footer.html")
  set(DOXYGEN_HTML_COLORSTYLE_HUE 220)
  set(DOXYGEN_HTML_COLORSTYLE_SAT 100)
  set(DOXYGEN_HTML_COLORSTYLE_GAMMA 80)
  set(DOXYGEN_GENERATE_TREEVIEW NO)
  set(DOXYGEN_USE_MATHJAX YES)
  # this target will only be built if specifically asked to.
  # run "make doc" to create the doxygen documentation
  doxygen_add_docs(
    doc
    ${PROJECT_SOURCE_DIR}
    COMMENT "Generate API-documents for NoteSearch."
  )
endif(DOXYGEN_FOUND)
