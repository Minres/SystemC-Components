package com.minres.scviewer.database;

import java.io.File;
import java.util.List;

public interface IWaveformDbLoader {
	
	public boolean load(IWaveformDb db, File inp) throws Exception;
	
	public Long getMaxTime();
	
	public List<IWaveform<? extends IWaveformEvent>> getAllWaves() ;
	
}
