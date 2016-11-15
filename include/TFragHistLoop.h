#ifndef _TFRAGHISTOGRAMLOOP_H_
#define _TFRAGHISTOGRAMLOOP_H_

/** \addtogroup Loops
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TFragHistLoop
/// 
/// This loop takes fragments and fills histograms based on
/// 'histos/MakeFragmentHistograms.cxx'
///
////////////////////////////////////////////////////////////////////////////////

#include <string>

#include "StoppableThread.h"
#include "TCompiledHistograms.h"
#include "ThreadsafeQueue.h"

class TFile;

class TFragHistLoop : public StoppableThread {
	public:
		static TFragHistLoop* Get(std::string name="");

		~TFragHistLoop();

#ifndef __CINT__
		std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment> > >& InputQueue() { return fInputQueue; }
#endif

		void SetOutputFilename(const std::string& name);
		std::string GetOutputFilename() const;

		void LoadLibrary(std::string library);
		std::string GetLibraryName() const;
		void ClearHistograms();

		void AddCutFile(TFile* cut_file);

		void Write();

		virtual void ClearQueue();

		TList* GetObjects();
		TList* GetGates();

		size_t GetItemsPopped()  { return 0; }
		size_t GetItemsPushed()  { return 0; }
		size_t GetItemsCurrent() { return 0; }
		size_t GetRate()         { return 0; }

	protected:
		bool Iteration();

	private:
		TFragHistLoop(std::string name);

		TCompiledHistograms fCompiledHistograms;

		void OpenFile();
		void CloseFile();

		TFile* fOutputFile;
		std::string fOutputFilename;

#ifndef __CINT__
		std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment> > > fInputQueue;
#endif

		ClassDef(TFragHistLoop,0);
};

/*! @} */
#endif /* _THISTOGRAMLOOP_H_ */
