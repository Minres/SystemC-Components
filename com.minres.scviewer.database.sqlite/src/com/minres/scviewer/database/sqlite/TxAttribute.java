/*******************************************************************************
 * Copyright (c) 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.database.sqlite;

import com.minres.scviewer.database.AssociationType;
import com.minres.scviewer.database.DataType;
import com.minres.scviewer.database.ITxAttribute;
import com.minres.scviewer.database.sqlite.tables.ScvTxAttribute;

public class TxAttribute implements ITxAttribute{

	Tx trTransaction;
	ScvTxAttribute scvAttribute;
	
	public TxAttribute(Tx trTransaction, ScvTxAttribute scvAttribute) {
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
