package com.itjw.txviewer.database;

public interface ITrRelation {

	RelationType getRelationType();
	
	ITransaction getSource();
	
	ITransaction getTarget();
}
