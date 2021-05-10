add_library(davinci STATIC
  davinci.cxx
  davinci.h
)
set_property(TARGET davinci PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(davinci
  collisions
  ${ITK_LIBRARIES}
  ${VTK_LIBRARIES}
  )

set_property(GLOBAL APPEND PROPERTY Slicer_TARGETS davinci)
