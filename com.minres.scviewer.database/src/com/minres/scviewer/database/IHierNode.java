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
package com.minres.scviewer.database;

import java.beans.PropertyChangeListener;
import java.util.List;

public interface IHierNode extends Comparable<IHierNode>{
	
	/**
	 * Attach a non-null PropertyChangeListener to this object.
	 * 
	 * @param l
	 *            a non-null PropertyChangeListener instance
	 * @throws IllegalArgumentException
	 *             if the parameter is null
	 */
	public void addPropertyChangeListener(PropertyChangeListener l);

	/**
	 * Remove a PropertyChangeListener from this component.
	 * 
	 * @param l
	 *            a PropertyChangeListener instance
	 */
	public void removePropertyChangeListener(PropertyChangeListener l) ;

	public String getFullName();
	
	public String getName();
	
	public void setName(String name);
	
	public void setParentName(String name);

	public List<IHierNode> getChildNodes();

}
