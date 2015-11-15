/*******************************************************************************
 * Copyright (c) 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.database.sqlite;

import java.beans.IntrospectionException;
import java.io.File;
import java.io.FileInputStream;
import java.lang.reflect.InvocationTargetException;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Collection;
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
	
	private List<RelationType> usedRelationsList = new ArrayList<>();

	private IWaveformDb db;
	
	private ScvSimProps scvSimProps;
		
	public SQLiteDbLoader() {
	}

	@Override
	public Long getMaxTime() {
		SQLiteDatabaseSelectHandler<ScvTxEvent> handler = new SQLiteDatabaseSelectHandler<ScvTxEvent>(ScvTxEvent.class,
				database, "time = (SELECT MAX(time) FROM ScvTxEvent)");
		try {
			List<ScvTxEvent> event = handler.selectObjects();
			if(event.size()>0)
				return event.get(0).getTime()*scvSimProps.getTime_resolution();
		} catch (SecurityException | IllegalArgumentException | InstantiationException | IllegalAccessException
				| InvocationTargetException | SQLException | IntrospectionException e) {
			e.printStackTrace();
		}
		return 0L;
	}

	@Override
	public List<IWaveform<? extends IWaveformEvent>> getAllWaves() {
		SQLiteDatabaseSelectHandler<ScvStream> handler = new SQLiteDatabaseSelectHandler<ScvStream>(ScvStream.class, database);
		List<IWaveform<? extends IWaveformEvent>> streams=new ArrayList<IWaveform<? extends IWaveformEvent>>();
		try {
			for(ScvStream scvStream:handler.selectObjects()){
				TxStream stream = new TxStream(database, db, scvStream);
				stream.setRelationTypeList(usedRelationsList);
				streams.add(stream);
			}
		} catch (SecurityException | IllegalArgumentException | InstantiationException | IllegalAccessException
				| InvocationTargetException | SQLException | IntrospectionException e) {
//			e.printStackTrace();
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
			for(ScvSimProps simProps:handler.selectObjects()){
				scvSimProps=simProps;
				database.setData("TIMERESOLUTION", scvSimProps.getTime_resolution());
			}
			return true;
		} catch (SecurityException | IllegalArgumentException | InstantiationException | IllegalAccessException
				| InvocationTargetException | SQLException | IntrospectionException e) {
			e.printStackTrace();
		}
		return false;
	}
	
	@Override
	public Collection<RelationType> getAllRelationTypes(){
		return usedRelationsList;
	}

}
