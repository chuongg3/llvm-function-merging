find_package(SQLite3 REQUIRED)
find_package(pybind11 REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

# Tensorflow Paths
set(TensorFlow_INCLUDE_DIRS "/usr/local/include/tensorflow")
set(TensorFlow_LIBRARIES "/usr/local/lib/libtensorflow.so" "/usr/local/lib/libtensorflow_framework.so")

include_directories(${TensorFlow_INCLUDE_DIRS})

add_llvm_component_library(LLVMipo
  AlwaysInliner.cpp
  Annotation2Metadata.cpp
  ArgumentPromotion.cpp
  Attributor.cpp
  AttributorAttributes.cpp
  BarrierNoopPass.cpp
  BlockExtractor.cpp
  CalledValuePropagation.cpp
  ConstantMerge.cpp
  CrossDSOCFI.cpp
  DeadArgumentElimination.cpp
  ElimAvailExtern.cpp
  EmbedBitcodePass.cpp
  ExtractGV.cpp
  ForceFunctionAttrs.cpp
  FunctionAttrs.cpp
  FunctionImport.cpp
  FunctionMerging.cpp
  FunctionSpecialization.cpp
  GlobalDCE.cpp
  GlobalOpt.cpp
  GlobalSplit.cpp
  HotColdSplitting.cpp
  IPO.cpp
  IROutliner.cpp
  InferFunctionAttrs.cpp
  Inliner.cpp
  Internalize.cpp
  LoopExtractor.cpp
  LowerTypeTests.cpp
  MemProfContextDisambiguation.cpp
  MergeFunctions.cpp
  ModuleInliner.cpp
  OpenMPOpt.cpp
  PartialInlining.cpp
  SampleContextTracker.cpp
  SampleProfile.cpp
  SampleProfileProbe.cpp
  SCCP.cpp
  StripDeadPrototypes.cpp
  StripSymbols.cpp
  SyntheticCountsPropagation.cpp
  ThinLTOBitcodeWriter.cpp
  WholeProgramDevirt.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/IPO
  ${pybind11_INCLUDE_DIRS}
  ${Python3_INCLUDE_DIRS}

  DEPENDS
  intrinsics_gen
  omp_gen

  COMPONENT_NAME
  IPO

  LINK_COMPONENTS
  AggressiveInstCombine
  Analysis
  BitReader
  BitWriter
  Core
  FrontendOpenMP
  InstCombine
  IRReader
  Linker
  Object
  ProfileData
  Scalar
  Support
  TargetParser
  TransformUtils
  Vectorize
  Instrumentation
  Option

  # Add SQLite3 to the linker libraries
  LINK_LIBS
  SQLite::SQLite3
  ${TensorFlow_LIBRARIES}
  )
