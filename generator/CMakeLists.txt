cmake_minimum_required(VERSION 3.11)

project(brutezone VERSION 1.0.0 LANGUAGES CSharp)

include(CSharpUtilities)

add_executable(${PROJECT_NAME}
	Program.cs
	Properties/AssemblyInfo.cs
)

find_program(NUGET nuget)
add_custom_target(nuget-restore
  COMMAND ${NUGET} restore ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.sln
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packages.config
${CMAKE_CURRENT_BINARY_DIR}/packages.config COPYONLY)

set_target_properties( ${PROJECT_NAME} PROPERTIES
  VS_GLOBAL_ROOTNAMESPACE ${PROJECT_NAME}
  VS_GLOBAL_ProjectGuid "{f84e584d-a422-3021-a20c-cec777413cae}"
  VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6.1"
  WIN32_EXECUTABLE TRUE
)

add_dependencies(${PROJECT_NAME} nuget-restore)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DOTNET_REFERENCES
	"Microsoft.CSharp"
	"NodaTime"
	"PresentationCore"
	"System"
	"System.Core"
	"System.Data"
	"System.Data.DataSetExtensions"
	"System.Net.Http"
	"System.Numerics"
	"System.Xml"
	"System.Xml.Linq"
)