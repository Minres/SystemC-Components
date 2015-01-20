package com.minres.scviewer.database.sqlite;

import java.beans.IntrospectionException;
import java.io.File;
import java.io.FileInputStream;
import java.lang.reflect.InvocationTargetException;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.database.IWaveformDbLoader;
import com.minres.scviewer.database.IWaveformEvent;
import com.minres.scviewer.database.RelationType;
import com.minres.scviewer.database.sqlite.db.IDatabase;
import com.minres.scviewer.database.sqlite.db.SQLiteDatabase;
import com.minres.scviewer.database.sqlite.db.SQLiteDatabaseSelectHandler;
import com.minres.scviewer.database.sqlite.tables.ScvSimProps;
import com.minres.scviewer.database.sqlite.tables.ScvStream;
import com.minres.scviewer.database.sqlite.tables.ScvTxEvent;

public class SQLiteDbLoader implements IWaveformDbLoader {

	protected IDatabase database;
	
	protected List<IWaveform<? extends IWaveformEvent>> streams;

	long timeResolution=1;

	private HashMap<String, RelationType> relationMap = new HashMap<String, RelationType>();

	private IWaveformDb db;
	
	public SQLiteDbLoader() {
	}

	@Override
	public Long getMaxTime() {
		SQLiteDatabaseSelectHandler<ScvTxEvent> handler = new SQLiteDatabaseSelectHandler<ScvTxEvent>(ScvTxEvent.class,
				database, "time = (SELECT MAX(time) FROM ScvTxEvent)");
		try {
			List<ScvTxEvent> event = handler.selectObjects();
			if(event.size()>0)
				return event.get(0).getTime();
		} catch (SecurityException | IllegalArgumentException | InstantiationException | IllegalAccessException
				| InvocationTargetException | SQLException | IntrospectionException e) {
			e.printStackTrace();
		}
		return 0L;
	}

	@Override
	public List<IWaveform<? extends IWaveformEvent>> getAllWaves() {
		if(streams==null){
			SQLiteDatabaseSelectHandler<ScvStream> handler = new SQLiteDatabaseSelectHandler<ScvStream>(ScvStream.class, database);
			streams=new ArrayList<IWaveform<? extends IWaveformEvent>>();
			try {
				for(ScvStream scvStream:handler.selectObjects()){
					streams.add(new TxStream(database, db, scvStream));
				}
			} catch (SecurityException | IllegalArgumentException | InstantiationException | IllegalAccessException
					| InvocationTargetException | SQLException | IntrospectionException e) {
	//			e.printStackTrace();
			}
		}
		return streams;
	}

	private byte[] x = "SQLite format 3".getBytes();

	@Override
	public boolean load(IWaveformDb db, File file) throws Exception {
		this.db=db;
		FileInputStream fis = new FileInputStream(file);
		byte[] buffer = new byte[x.length];
		int read = fis.read(buffer, 0, x.length);
		fis.close();
		if (read == x.length)
			for (int i = 0; i < x.length; i++)
				if (buffer[i] != x[i])	return false;

		database=new SQLiteDatabase(file.getAbsolutePath());
		database.setData("TIMERESOLUTION", 1L);
		SQLiteDatabaseSelectHandler<ScvSimProps> handler = new SQLiteDatabaseSelectHandler<ScvSimProps>(ScvSimProps.class, database);
		try {
			for(ScvSimProps scvSimProps:handler.selectObjects()){
				database.setData("TIMERESOLUTION", scvSimProps.getTime_resolution());
			}
			return true;
		} catch (SecurityException | IllegalArgumentException | InstantiationException | IllegalAccessException
				| InvocationTargetException | SQLException | IntrospectionException e) {
			e.printStackTrace();
		}
		return false;
	}


	public RelationType getRelationType(String relationName) {
		if(relationMap.containsKey(relationName)) return relationMap.get(relationName);
		RelationType type = new RelationType(relationName);
		relationMap.put(relationName, type);
		return type;
	}

}
