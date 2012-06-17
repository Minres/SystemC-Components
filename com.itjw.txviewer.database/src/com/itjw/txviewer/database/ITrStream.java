package com.itjw.txviewer.database;

import java.util.List;

public interface  ITrStream extends ITrHierNode {

	public Long getId();

	public String getKind();

	public ITrDb getDb();

	public List<ITrGenerator> getGenerators();

	public List<ITransaction> getTransactions();

	public int getMaxConcurrrentTx();

}
