package com.minres.scviewer.database.sqlite;

import java.beans.IntrospectionException;
import java.lang.reflect.InvocationTargetException;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.NavigableSet;
import java.util.TreeSet;

import com.minres.scviewer.database.HierNode;
import com.minres.scviewer.database.ITxGenerator;
import com.minres.scviewer.database.ITxStream;
import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.database.sqlite.db.IDatabase;
import com.minres.scviewer.database.sqlite.db.SQLiteDatabaseSelectHandler;
import com.minres.scviewer.database.sqlite.tables.ScvGenerator;
import com.minres.scviewer.database.sqlite.tables.ScvStream;
import com.minres.scviewer.database.sqlite.tables.ScvTx;

public class TxStream extends HierNode implements ITxStream {

	private IDatabase database;

	private String fullName;
	
	private IWaveformDb db;
	
	private ScvStream scvStream;
	
	private HashMap<Integer, TxGenerator> generators;
	
	private NavigableSet<ITx> transactions;
	
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
			generators=new HashMap<Integer, TxGenerator>();
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
	public NavigableSet<ITx> getTransactions() {
		checkTransactions();
		return transactions;
	}

	@Override
	public ITx getTransactionById(long id) {
		checkTransactions();
		for(ITx trans:transactions){
			if(trans.getId()==id)
				return trans;
		}
		return null;
	}

	protected void checkTransactions() {
		if(transactions==null){
			if(generators==null) getGenerators();
			SQLiteDatabaseSelectHandler<ScvTx> handler = new SQLiteDatabaseSelectHandler<ScvTx>(ScvTx.class, database,
					"stream="+scvStream.getId());
			transactions=new TreeSet<ITx>();
			try {
				for(ScvTx scvTx:handler.selectObjects()){
					transactions.add(new Tx(database, this, generators.get(scvTx.getGenerator()), scvTx));
				}
			} catch (SecurityException | IllegalArgumentException | InstantiationException | IllegalAccessException
					| InvocationTargetException | SQLException | IntrospectionException e) {
				e.printStackTrace();
			}
		}
	}

}
