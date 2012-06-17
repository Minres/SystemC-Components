package com.itjw.txviewer.database;

import java.util.List;

public interface ITrGenerator {
	public Long getId();
	public ITrStream getStream();
	public String getName();
//	public Boolean isActive();
	public List<ITransaction> getTransactions();
}
