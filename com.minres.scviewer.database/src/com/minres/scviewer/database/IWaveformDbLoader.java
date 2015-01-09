package com.minres.scviewer.database;

import java.io.File;
import java.util.List;

public interface IWaveformDbLoader {
	
	public boolean load(IWaveformDb db, File inp) throws Exception;
	
	public EventTime getMaxTime();
	
	public List<IWaveform> getAllWaves() ;
	
}
