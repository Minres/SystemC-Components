package com.minres.scviewer.database.vcd;

import java.io.File;
import java.io.FileInputStream;

import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.database.IWaveformDbFactory;

public class VCDDbFactory implements IWaveformDbFactory {

	private byte[] x = "$date".getBytes();

	public VCDDbFactory(){
		
	}
	
	@Override
	public IWaveformDb createDatabase(File file) {
		try {
			FileInputStream fis = new FileInputStream(file);
			byte[] buffer = new byte[x.length];
			int read = fis.read(buffer, 0, x.length);
			fis.close();
			if (read == x.length)
				for (int i = 0; i < x.length; i++)
					if (buffer[i] != x[i])
						return null;
			VCDDb db = new VCDDb();
			db.load(file);
			return db;
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
	}

}
