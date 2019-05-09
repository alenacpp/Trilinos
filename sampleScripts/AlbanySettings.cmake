#Typical options needed to build the "full" version of Albany
#

# Time out all tests at 600 seconds
SET(DART_TESTING_TIMEOUT "600" CACHE STRING "" )

# Boost is needed for Albany - Turn it on
SET(TPL_ENABLE_Boost ON CACHE BOOL "" )
SET(TPL_ENABLE_BoostLib ON CACHE BOOL "" )
SET(TPL_ENABLE_BoostAlbLib ON CACHE BOOL "" )

SET(Zoltan_ENABLE_ULONG_IDS ON CACHE BOOL "" )
SET(Tpetra_INST_FLOAT OFF CACHE BOOL "" )
SET(Tpetra_INST_COMPLEX_FLOAT OFF CACHE BOOL "" )
SET(Tpetra_INST_COMPLEX_DOUBLE OFF CACHE BOOL "" )

SET(Trilinos_ENABLE_TESTS OFF CACHE BOOL "" )
SET(Trilinos_ENABLE_EXAMPLES OFF CACHE BOOL "" )
SET(Trilinos_ENABLE_EXPORT_MAKEFILES OFF CACHE BOOL "" )
SET(Trilinos_ASSERT_MISSING_PACKAGES OFF CACHE BOOL "" )
SET(CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "" )
SET(Trilinos_VERBOSE_CONFIGURE OFF CACHE BOOL "" )

SET(Trilinos_ENABLE_ALL_PACKAGES OFF CACHE BOOL "" )
SET(Trilinos_ENABLE_ALL_OPTIONAL_PACKAGES OFF CACHE BOOL "" )
SET(Trilinos_ENABLE_SECONDARY_TESTED_CODE ON CACHE BOOL "" )

SET(Trilinos_ENABLE_ThyraTpetraAdapters ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Ifpack2 ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Amesos2 ON CACHE BOOL "" )
SET(Amesos2_ENABLE_KLU2 ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Zoltan2 ON CACHE BOOL "" )
SET(Trilinos_ENABLE_MueLu ON CACHE BOOL "" )

SET(Trilinos_ENABLE_Teuchos ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Shards ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Sacado ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Epetra ON CACHE BOOL "" )
SET(Trilinos_ENABLE_EpetraExt ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Ifpack ON CACHE BOOL "" )
SET(Trilinos_ENABLE_AztecOO ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Amesos ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Anasazi ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Belos ON CACHE BOOL "" )
SET(Trilinos_ENABLE_ML ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Phalanx ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Intrepid ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Intrepid2 ON CACHE BOOL "" )
SET(Trilinos_ENABLE_NOX ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Stratimikos ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Thyra ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Rythmos ON CACHE BOOL "" )
SET(Trilinos_ENABLE_OptiPack OFF CACHE BOOL "" )
SET(Trilinos_ENABLE_GlobiPack OFF CACHE BOOL "" )
SET(Trilinos_ENABLE_Stokhos ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Isorropia ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Piro ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Teko ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Zoltan ON CACHE BOOL "" )
SET(Trilinos_ENABLE_Pamgen ON CACHE BOOL "" )

SET(Trilinos_ENABLE_MOOCHO OFF CACHE BOOL "" )
SET(Trilinos_ENABLE_Mesquite OFF CACHE BOOL "" )
SET(Trilinos_ENABLE_FEI OFF CACHE BOOL "" )
SET(Trilinos_ENABLE_PyTrilinos OFF CACHE BOOL "" )
SET(TPL_ENABLE_Matio OFF CACHE BOOL "" )
SET(Teuchos_ENABLE_COMPLEX OFF CACHE BOOL "" )

SET(ZOLTAN_BUILD_ZFDRIVE OFF CACHE BOOL "" )

SET(Phalanx_ENABLE_TEUCHOS_TIME_MONITOR ON CACHE BOOL "" )
SET(Stokhos_ENABLE_TEUCHOS_TIME_MONITOR ON CACHE BOOL "" )
SET(Stratimikos_ENABLE_TEUCHOS_TIME_MONITOR ON CACHE BOOL "" )

# Seacas
SET(Trilinos_ENABLE_SEACAS ON CACHE BOOL "" )
SET(Trilinos_ENABLE_SEACASIoss ON CACHE BOOL "" )
SET(Trilinos_ENABLE_SEACASExodus ON CACHE BOOL "" )

# STK
SET(Trilinos_ENABLE_STK ON CACHE BOOL "" )
SET(Trilinos_ENABLE_STKClassic OFF CACHE BOOL "" )
SET(Trilinos_ENABLE_STKUtil ON CACHE BOOL "" )
SET(Trilinos_ENABLE_STKTopology ON CACHE BOOL "" )
SET(Trilinos_ENABLE_STKMesh ON CACHE BOOL "" )
SET(Trilinos_ENABLE_STKIO ON CACHE BOOL "" )
SET(Trilinos_ENABLE_STKSearch OFF CACHE BOOL "" )
SET(Trilinos_ENABLE_STKSearchUtil OFF CACHE BOOL "" )
SET(Trilinos_ENABLE_STKTransfer ON CACHE BOOL "" )
SET(Trilinos_ENABLE_STKUnit_tests OFF CACHE BOOL "" )
SET(Trilinos_ENABLE_STKDoc_tests OFF CACHE BOOL "" )
SET(Trilinos_ENABLE_STKExp OFF CACHE BOOL "" )
