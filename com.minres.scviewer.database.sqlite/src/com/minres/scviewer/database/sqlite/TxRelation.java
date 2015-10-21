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
