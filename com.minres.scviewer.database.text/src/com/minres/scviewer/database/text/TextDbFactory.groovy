package com.minres.scviewer.database.text

import java.io.FileInputStream;

import com.minres.scviewer.database.ITrDb
import com.minres.scviewer.database.ITransactionDbFactory;

class TextDbFactory implements ITransactionDbFactory {

	byte[] x = "scv_tr_stream".bytes
	
	@Override
	public ITrDb createDatabase(File file) {
		try {
			FileInputStream fis = new FileInputStream(file);
            byte[] buffer = new byte[x.size()];
            def read = fis.read(buffer, 0, x.size());
            fis.close();
            if(read==x.size())
            	for(int i=0; i<x.size(); i++)
            		if(buffer[i]!=x[i]) return null;
			def db = new TextDb();
			db.load(file)
			return db
		} catch (Exception e) {
			e.printStackTrace()
		}
		return null;
	}

}
