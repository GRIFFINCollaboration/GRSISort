void GRIFFIN_PACES_AngleCalc(){
	printf("Calculates expected electron-gamma coincidence angles\n");
	printf("_____________________________________________________\n");
	//USER defs::
	double dist = 110;	//HPGe distance (mm)
	double angleBin=5.0;	//bin width, degrees
	int pstart=0,pend=5;	//range of PACES detectors to include (0-5 for all).
        bool table = true;	//FILE FORMART:: true = table output, false = angle indexes ONLY.
	//END USER defs.
   	std::vector<std::pair<double, int>> eg, storeA;
	double a, ap, x=0, y=0;
	int b, gr=0;
	const char* name = "GRIFFIN_PACES_AngleGroups.dat";
        FILE *ascii = fopen(name,"w");

   	for (int gd = 1; gd <= 16; ++gd){
		if(gd==13) continue;
      		for (int gc = 0; gc < 4; ++gc){
			for (int p = pstart; p < pend; ++p){
				//index assigned 0-319 based on HPGe and PACES det#
				double angle = TGriffin::GetPosition(gd, gc, dist).Angle(TPaces::GetPosition(p))*180./TMath::Pi();
				eg.push_back(std::make_pair(angle, ((64*p)+(((gd-1)*4)+gc))));
	 		}
      		}
   	}
	
	int size = int(eg.size());
	printf("Found %i individual PACES-GRIFFIN angles (including %i PACES detector(s))\n",size,(size/60));
	//'sort' will default to sorting by the key (angle in degrees) which is OK here.
   	std::sort(eg.begin(), eg.end());

	//GROUPING
	fprintf(ascii,"Group\tAngle\tIndex\n");
	fprintf(ascii,"-----\tDegr.\t-----\n");
        if(size!=0) ap=eg[0].first;

	for(int i=0;i<size;++i){
	   a=eg[i].first; b=eg[i].second;
	   if(a<=(ap+angleBin)){
	      x++;
	      storeA.push_back(std::make_pair(a, b));
	      if(table) fprintf(ascii,"%i\t%.2f\t%i\n",gr,a,b);
	      if(!table) fprintf(ascii,"%i, ",b);

	   } else if(a>(ap+angleBin)){
	      y++; gr++; storeA.clear();
	      storeA.push_back(std::make_pair(a, b));
	      if(table) fprintf(ascii,"%i\t%.2f\t%i\n",gr,a,b); //def.
	      if(!table){ fprintf(ascii,"\n"); fprintf(ascii,"%i, ",b); }
	   }
	   ap = storeA[0].first;
	   //printf("%.0f/%.0f::%.2f/%.2f\n",x,y,a,ap);	//diagnostic
	   x=y=0;
	}
	printf("Found %i PACES-GRIFFIN angle groups. For suggested grouping see ./%s\n",gr+1,name);
	fclose(ascii);
}
