add_executable(EadorUnitTests
  main.cpp
  ArrayInclusiveEndTests.cpp
  AsBinaryTests.cpp
  OptionalTests.cpp
  ProvinceTests.cpp
  RangeTests.cpp
)

# ProvinceTests uses dcProvince/dcHero/dcUnit, so the tests link the game code.
target_link_libraries(EadorUnitTests PRIVATE eador_core)

add_test(NAME EadorUnitTests COMMAND EadorUnitTests)
