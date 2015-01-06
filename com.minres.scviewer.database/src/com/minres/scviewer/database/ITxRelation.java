package com.minres.scviewer.database;

public interface ITxRelation {

	RelationType getRelationType();
	
	ITx getSource();
	
	ITx getTarget();
}
