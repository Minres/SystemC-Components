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
import java.lang.reflect.InvocationTargetException;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.TreeMap;
import java.util.Vector;

import com.minres.scviewer.database.HierNode;
import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.ITxEvent;
import com.minres.scviewer.database.ITxGenerator;
import com.minres.scviewer.database.ITxStream;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.database.IWaveformEvent;
import com.minres.scviewer.database.RelationType;
import com.minres.scviewer.database.sqlite.db.IDatabase;
import com.minres.scviewer.database.sqlite.db.SQLiteDatabaseSelectHandler;
import com.minres.scviewer.database.sqlite.tables.ScvGenerator;
import com.minres.scviewer.database.sqlite.tables.ScvStream;
import com.minres.scviewer.database.sqlite.tables.ScvTx;

public class TxStream extends HierNode implements ITxStream<ITxEvent> {

	private IDatabase database;

	private String fullName;
	
	private IWaveformDb db;
	
	private ScvStream scvStream;
	
	private TreeMap<Integer, TxGenerator> generators;
	
	private TreeMap<Integer, ITx> transactions;
	
	private Integer maxConcurrency;
	
	private TreeMap<Long, List<ITxEvent>> events;

	private List<RelationType> usedRelationsList;
	
	public TxStream(IDatabase database, IWaveformDb waveformDb, ScvStream scvStream) {
		super(scvStream.getName());
		this.database=database;
		fullName=scvStream.getName();
		this.scvStream=scvStream;
		db=waveformDb;
	}

	@Override
	public IWaveformDb getDb() {
		return db;
	}

	@Override
	public String getFullName() {
		return fullName;
	}

	@Override
	public Long getId() {
		return (long) scvStream.getId();
	}

	@Override
	public String getKind() {
		return scvStream.getKind();
	}

	@Override
	public List<ITxGenerator> getGenerators() {
		if(generators==null){
			SQLiteDatabaseSelectHandler<ScvGenerator> handler = new SQLiteDatabaseSelectHandler<ScvGenerator>(
					ScvGenerator.class, database, "stream="+scvStream.getId());
			generators=new TreeMap<Integer, TxGenerator>();
			try {
				for(ScvGenerator scvGenerator:handler.selectObjects()){
					generators.put(scvGenerator.getId(), new TxGenerator(this, scvGenerator));
				}
			} catch (SecurityException | IllegalArgumentException | InstantiationException | IllegalAccessException
					| InvocationTargetException | SQLException | IntrospectionException e) {
				e.printStackTrace();
			}
		}
		return new ArrayList<ITxGenerator>(generators.values());
	}

	@Override
	public int getMaxConcurrency() {
		if(maxConcurrency==null){
			java.sql.Connection connection=null;
			java.sql.Statement statement=null;
			java.sql.ResultSet resultSet=null;
			try {
				connection = database.createConnection();
				statement = connection.createStatement();
				StringBuilder sb = new StringBuilder();
				sb.append("SELECT MAX(concurrencyLevel) as concurrencyLevel FROM ScvTx where stream=");
				sb.append(scvStream.getId());
				resultSet = statement.executeQuery(sb.toString());
				while (resultSet.next()) {
					Object value = resultSet.getObject("concurrencyLevel");
					if(value!=null)
						maxConcurrency=(Integer) value;
				}
			} catch (SQLException e) {
				if(maxConcurrency==null) maxConcurrency=0;
			} finally {
				try{
				if(resultSet!=null) resultSet.close();
				if(statement!=null) statement.close();
				if(connection!=null) connection.close();
				} catch (SQLException e) {	}
			}
			maxConcurrency+=1;
		}
		return maxConcurrency;
	}

	@Override
	public  NavigableMap<Long, List<ITxEvent>> getEvents(){
		if(events==null){
			events=new TreeMap<Long, List<ITxEvent>>();
			for(Entry<Integer, ITx> entry:getTransactions().entrySet()){
				putEvent(new TxEvent(TxEvent.Type.BEGIN, entry.getValue()));
				putEvent(new TxEvent(TxEvent.Type.END, entry.getValue()));
			}	
		}
		return events;
	}

	private void putEvent(TxEvent ev){
		Long time = ev.getTime();
		if(!events.containsKey(time)){
			Vector<ITxEvent> vector=new Vector<ITxEvent>();
			vector.add(ev);
			events.put(time,  vector);
		} else {
			events.get(time).add(ev);
		}
	}

	protected Map<Integer, ITx> getTransactions() {
		if(transactions==null){
			if(generators==null) getGenerators();
			transactions = new TreeMap<Integer, ITx>();
			SQLiteDatabaseSelectHandler<ScvTx> handler = new SQLiteDatabaseSelectHandler<ScvTx>(ScvTx.class, database,
					"stream="+scvStream.getId());
			try {
				for(ScvTx scvTx:handler.selectObjects()){
					transactions.put(scvTx.getId(), new Tx(database, this, generators.get(scvTx.getGenerator()), scvTx));
				}
			} catch (SecurityException | IllegalArgumentException | InstantiationException | IllegalAccessException
					| InvocationTargetException | SQLException | IntrospectionException e) {
				e.printStackTrace();
			}
		}
		return transactions;
	}

	@Override
	public Collection<ITxEvent> getWaveformEventsAtTime(Long time) {
		return getEvents().get(time);
	}

	public void setRelationTypeList(List<RelationType> usedRelationsList){
		this.usedRelationsList=usedRelationsList;
	}
	
	public RelationType getRelationType(String name) {
		RelationType relType=RelationType.create(name);
		if(!usedRelationsList.contains(relType)) usedRelationsList.add(relType);
		return relType;
	}

	@Override
	public Boolean equals(IWaveform<? extends IWaveformEvent> other) {
		return(other instanceof TxStream && this.getId()==other.getId());
	}

}
