package com.minres.scviewer.database;

public interface ITrRelation {

	RelationType getRelationType();
	
	ITransaction getSource();
	
	ITransaction getTarget();
}
