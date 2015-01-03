package com.minres.scviewer.database.sqlite;

import com.minres.scviewer.database.AssociationType;
import com.minres.scviewer.database.DataType;
import com.minres.scviewer.database.ITrAttribute;
import com.minres.scviewer.database.sqlite.tables.ScvTxAttribute;

public class Attribute implements ITrAttribute{

	Transaction trTransaction;
	ScvTxAttribute scvAttribute;
	
	public Attribute(Transaction trTransaction, ScvTxAttribute scvAttribute) {
		this.trTransaction=trTransaction;
		this.scvAttribute=scvAttribute;
	}

	@Override
	public String getName() {
		return scvAttribute.getName();
	}

	@Override
	public DataType getDataType() {
		return DataType.values()[scvAttribute.getData_type()];
	}

	@Override
	public AssociationType getType() {
		return AssociationType.values()[scvAttribute.getType()];
	}

	@Override
	public Object getValue() {
		return scvAttribute.getData_value();
	}

}
