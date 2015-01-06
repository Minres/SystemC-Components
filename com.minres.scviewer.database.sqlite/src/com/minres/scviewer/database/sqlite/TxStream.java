package com.minres.scviewer.database.sqlite;

import java.beans.IntrospectionException;
import java.lang.reflect.InvocationTargetException;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.minres.scviewer.database.HierNode;
import com.minres.scviewer.database.ITxGenerator;
import com.minres.scviewer.database.ITxStream;
import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.sqlite.db.SQLiteDatabaseSelectHandler;
import com.minres.scviewer.database.sqlite.tables.ScvGenerator;
import com.minres.scviewer.database.sqlite.tables.ScvStream;
import com.minres.scviewer.database.sqlite.tables.ScvTx;

public class TxStream extends HierNode implements ITxStream {

	private String fullName;
	
	private SQLiteDb db;
	
	private ScvStream scvStream;
	
	private HashMap<Integer, TxGenerator> generators;
	
	private List<ITx> transactions;
	
	public TxStream(SQLiteDb trSQLiteDb, ScvStream scvStream) {
		super(scvStream.getName());
		fullName=scvStream.getName();
		this.scvStream=scvStream;
		db=trSQLiteDb;
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
	public SQLiteDb getDb() {
		return db;
	}

	@Override
	public List<ITxGenerator> getGenerators() {
		if(generators==null){
			SQLiteDatabaseSelectHandler<ScvGenerator> handler = new SQLiteDatabaseSelectHandler<ScvGenerator>(
					ScvGenerator.class, db.getDb(), "stream="+scvStream.getId());
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
	public List<ITx> getTransactions() {
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
			SQLiteDatabaseSelectHandler<ScvTx> handler = new SQLiteDatabaseSelectHandler<ScvTx>(ScvTx.class, db.getDb(),
					"stream="+scvStream.getId());
			transactions=new ArrayList<ITx>();
			try {
				for(ScvTx scvTx:handler.selectObjects()){
					transactions.add(new Tx(this, generators.get(scvTx.getGenerator()), scvTx));
				}
			} catch (SecurityException | IllegalArgumentException | InstantiationException | IllegalAccessException
					| InvocationTargetException | SQLException | IntrospectionException e) {
				e.printStackTrace();
			}
		}
	}

}
