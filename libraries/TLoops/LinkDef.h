// TDataLoop.h StoppableThread.h TFragWriteLoop.h TTerminalLoop.h TFragment.h TEventBuildingLoop.h TDetBuildingLoop.h TAnalysisWriteLoop.h TRnTupleWriteLoop.h TFragHistLoop.h TCompiledHistograms.h TRuntimeObjects.h TAnalysisHistLoop.h

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link         C++ nestedclasses;

#pragma link C++ class StoppableThread + ;
#pragma link C++ class std::vector < StoppableThread*> + ;

#pragma link C++ class TDataLoop + ;
#pragma link C++ class TFragWriteLoop + ;

#pragma link C++ class TEventBuildingLoop + ;
#pragma link C++ class TDetBuildingLoop + ;
#pragma link C++ class TAnalysisWriteLoop + ;
#pragma link C++ class TRnTupleWriteLoop + ;

#pragma link C++ class TCompiledHistograms + ;
#pragma link C++ class TRuntimeObjects + ;
#pragma link C++ class TFragHistLoop + ;
#pragma link C++ class TAnalysisHistLoop + ;

#endif
