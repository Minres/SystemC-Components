package com.minres.scviewer.database.text

import com.minres.scviewer.database.ITxRelation
import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.RelationType;

class TxRelation implements ITxRelation {
	Tx source
	
	Tx target
	
	RelationType relationType
	
	
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
