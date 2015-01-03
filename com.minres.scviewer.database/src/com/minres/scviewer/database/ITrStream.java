/*******************************************************************************
 * Copyright (c) 2012 IT Just working.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     IT Just working - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.database;

import java.util.List;

public interface  ITrStream extends ITrHierNode {

	public Long getId();

	public String getKind();

	public ITrDb getDb();

	public List<ITrGenerator> getGenerators();

	public List<ITransaction> getTransactions();

	public ITransaction getTransactionById(long id);

}
