package com.minres.scviewer.database.sqlite;

import java.io.File;
import java.io.FileInputStream;

import com.minres.scviewer.database.ITrDb;
import com.minres.scviewer.database.ITransactionDbFactory;

public class SQLiteDbFactory implements ITransactionDbFactory {

	private byte[] x = "SQLite format 3".getBytes();

	public SQLiteDbFactory() {
	}

	@Override
	public ITrDb createDatabase(File file) {
		try {
			FileInputStream fis = new FileInputStream(file);
			byte[] buffer = new byte[x.length];
			int read = fis.read(buffer, 0, x.length);
			fis.close();
			if (read == x.length)
				for (int i = 0; i < x.length; i++)
					if (buffer[i] != x[i])
						return null;
			SQLiteDb db = new SQLiteDb();
			db.load(file);
			return db;
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
	}

}
