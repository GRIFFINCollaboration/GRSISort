
   GetOutputList()->Add(new TH1D("Charge_0x0000","Charge_0x0000",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x0001","Charge_0x0001",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x0002","Charge_0x0002",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x0003","Charge_0x0003",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x0004","Charge_0x0004",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x0005","Charge_0x0005",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x0006","Charge_0x0006",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x0007","Charge_0x0007",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x0008","Charge_0x0008",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x0009","Charge_0x0009",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x000a","Charge_0x000a",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x000b","Charge_0x000b",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x000c","Charge_0x000c",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x000d","Charge_0x000d",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x000e","Charge_0x000e",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x000f","Charge_0x000f",8000,0,4000));


   GetOutputList()->Add(new TH1D("Charge_0x1000","Charge_0x1000",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x1001","Charge_0x1001",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x1002","Charge_0x1002",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x1003","Charge_0x1003",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x1004","Charge_0x1004",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x1005","Charge_0x1005",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x1006","Charge_0x1006",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x1007","Charge_0x1007",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x1008","Charge_0x1008",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x1009","Charge_0x1009",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x100a","Charge_0x100a",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x100b","Charge_0x100b",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x100c","Charge_0x100c",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x100d","Charge_0x100d",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x100e","Charge_0x100e",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_0x100f","Charge_0x100f",8000,0,4000));
   GetOutputList()->Add(new TH1D("EnergySum","EnergySum",5000,0,2500));

   GetOutputList()->Add(new TH1D("Charge_nofilter","Charge_nofilter",8000,0,4000));
   GetOutputList()->Add(new TH1D("Charge_filter","Charge_filter",8000,0,4000));
   GetOutputList()->Add(new TH1D("TriggerPattern","TriggerPattern",256,0,256));

   GetOutputList()->Add(new TH1D("Cfd_0x100f","Timestamp_0x100f",4000,0,4000));

   GetOutputList()->Add(new TH1D("PPG","PPG",100000,0,100000));
   GetOutputList()->Add(new TH1D("ChannelId","ChannelId",1000000,0,1000000));
   GetOutputList()->Add(new TH1D("MidasId","MidasId",100000,0,100000));
   GetOutputList()->Add(new TH1D("MidasTimeStamp","MidasTimeStamp",1000000,0,1000000));	
   GetOutputList()->Add(new TH1D("TriggerId","TriggerId",1000000,6000000,10000000));

   GetOutputList()->Add(new TH2D("hp_charge","Channel vs Charge",64,0,64,24000,0,12000));
   GetOutputList()->Add(new TH2D("hp_energy","Channel vs Energy",64,0,64,10000,0,2500));
   
   GetOutputList()->Add(new TH1D("test","test",128,-64,64));


