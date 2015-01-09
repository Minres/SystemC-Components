package com.minres.scviewer.database.sqlite;

import java.beans.IntrospectionException;
import java.io.File;
import java.lang.reflect.InvocationTargetException;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.minres.scviewer.database.EventTime;
import com.minres.scviewer.database.HierNode;
import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.database.IHierNode;
import com.minres.scviewer.database.ITxStream;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.InputFormatException;
import com.minres.scviewer.database.RelationType;
import com.minres.scviewer.database.sqlite.db.IDatabase;
import com.minres.scviewer.database.sqlite.db.SQLiteDatabase;
import com.minres.scviewer.database.sqlite.db.SQLiteDatabaseSelectHandler;
import com.minres.scviewer.database.sqlite.tables.ScvSimProps;
import com.minres.scviewer.database.sqlite.tables.ScvStream;
import com.minres.scviewer.database.sqlite.tables.ScvTxEvent;

public class SQLiteDb extends HierNode implements IWaveformDb {

	protected IDatabase database;
	
	protected List<IWaveform> streams;

	long timeResolution=1;

	private HashMap<String, RelationType> relationMap = new HashMap<String, RelationType>();
	
	IDatabase getDb(){
		return database;
	}
	
	public SQLiteDb() {
		super("SQLiteDb");
	}

	@Override
	public EventTime getMaxTime() {
		SQLiteDatabaseSelectHandler<ScvTxEvent> handler = new SQLiteDatabaseSelectHandler<ScvTxEvent>(ScvTxEvent.class,
				database, "time = SELECT MAX(time) FROM ScvTxEvent");
		try {
			List<ScvTxEvent> event = handler.selectObjects();
			if(event.size()>0)
				return new EventTime(event.get(0).getTime(), "fs");
		} catch (SecurityException | IllegalArgumentException | InstantiationException | IllegalAccessException
				| InvocationTargetException | SQLException | IntrospectionException e) {
			e.printStackTrace();
		}
		return new EventTime(0L, "s");
	}

	@Override
	public List<IWaveform> getAllWaves() {
		if(streams==null){
			SQLiteDatabaseSelectHandler<ScvStream> handler = new SQLiteDatabaseSelectHandler<ScvStream>(ScvStream.class, database);
			streams=new ArrayList<IWaveform>();
			try {
				for(ScvStream scvStream:handler.selectObjects()){
					streams.add(new TxStream(this, scvStream));
				}
			} catch (SecurityException | IllegalArgumentException | InstantiationException | IllegalAccessException
					| InvocationTargetException | SQLException | IntrospectionException e) {
	//			e.printStackTrace();
			}
		}
		return streams;
	}

	@Override
	public void load(File file) throws InputFormatException {
		database=new SQLiteDatabase(file.getAbsolutePath());
		SQLiteDatabaseSelectHandler<ScvSimProps> handler = new SQLiteDatabaseSelectHandler<ScvSimProps>(ScvSimProps.class, database);
		try {
			for(ScvSimProps scvSimProps:handler.selectObjects()){
				timeResolution=scvSimProps.getTime_resolution();
			}
		} catch (SecurityException | IllegalArgumentException | InstantiationException | IllegalAccessException
				| InvocationTargetException | SQLException | IntrospectionException e) {
			e.printStackTrace();
		}
		buildHierarchyNodes();
	}

	@Override
	public void clear() {
		database=null;
	}

	@Override
	public IWaveform getStreamByName(String name) {
		for (IWaveform n : getAllWaves())
			if (n.getFullName().equals(name))
				return n;
		return null;
	}

	public ITxStream getStreamById(long id) {
		for (IWaveform n : getAllWaves()) 
			if (n.getId().equals(id))
				return (ITxStream) n;
		return null;
	}

	private void buildHierarchyNodes() throws InputFormatException{
		for(IWaveform stream:getAllWaves()){
			String[] hier = stream.getFullName().split("\\.");
			IHierNode node = this;
			for(String name:hier){
				IHierNode n1 = null;
				 for (IHierNode n : node.getChildNodes()) {
				        if (n.getName().equals(name)) {
				            n1=n;
				            break;
				        }
				    }
				if(name == hier[hier.length-1]){ //leaf
					if(n1!=null) {
						if(n1 instanceof HierNode){
							node.getChildNodes().remove(n1);
							stream.getChildNodes().addAll(n1.getChildNodes());
						} else {
							throw new InputFormatException();
						}
					}
					stream.setName(name);
					node.getChildNodes().add(stream);
					node=stream;
				} else { // intermediate
					if(n1 != null) {
						node=n1;
					} else {
						HierNode newNode = new HierNode(name);
						node.getChildNodes().add(newNode);
						node=newNode;
					}
				}
			}
		}
	}

	public RelationType getRelationType(String relationName) {
		if(relationMap.containsKey(relationName)) return relationMap.get(relationName);
		RelationType type = new RelationType(relationName);
		relationMap.put(relationName, type);
		return type;
	}

}
