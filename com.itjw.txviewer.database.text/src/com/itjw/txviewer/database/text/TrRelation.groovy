package com.itjw.txviewer.database.text

import com.itjw.txviewer.database.ITrRelation
import com.itjw.txviewer.database.ITransaction;
import com.itjw.txviewer.database.RelationType;

class TrRelation implements ITrRelation {
	Transaction source
	
	Transaction target
	
	RelationType relationType
	
	
	public TrRelation(RelationType relationType, Transaction source, Transaction target) {
		this.source = source;
		this.target = target;
		this.relationType = relationType;
	}

	@Override
	public RelationType getRelationType() {
		return relationType;
	}

	@Override
	public ITransaction getSource() {
		return source;
	}

	@Override
	public ITransaction getTarget() {
		return target;
	}

}
