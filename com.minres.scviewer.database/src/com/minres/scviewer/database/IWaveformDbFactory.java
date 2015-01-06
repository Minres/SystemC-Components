package com.minres.scviewer.database;

import java.io.File;

public interface IWaveformDbFactory {
	
	IWaveformDb createDatabase(File file);

}
