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
package com.itjw.txviewer.database;

import java.util.Collection;
import java.util.List;
import java.util.Set;

public interface ITransaction {

	public Long getId();
	
	public ITrStream getStream();
	
	public ITrGenerator getGenerator();
	
	public EventTime getBeginTime();
	
	public EventTime getEndTime();
	
	public List<ITrAttribute> getAttributes();
	
	public Collection<ITrRelation> getIncomingRelations();

	public Collection<ITrRelation> getOutgoingRelations();
}
