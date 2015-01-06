package com.minres.scviewer.database.sqlite;

import com.minres.scviewer.database.ITxRelation;
import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.RelationType;

public class TxRelation implements ITxRelation {

	RelationType relationType;
	Tx source, target;

	public TxRelation(RelationType relationType, Tx source, Tx target) {
		this.source = source;
		this.target = target;
		this.relationType = relationType;
	}

	@Override
	public RelationType getRelationType() {
		return relationType;
	}

	@Override
	public ITx getSource() {
		return source;
	}

	@Override
	public ITx getTarget() {
		return target;
	}

}
