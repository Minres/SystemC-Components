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
package com.minres.scviewer.database.vcd;

import com.minres.scviewer.database.BitVector;

/**
 * The Interface IVCDDatabaseBuilder. It allows to add VCD events into the database
 */
public interface IVCDDatabaseBuilder {

	/**
	 * Enter module.
	 *
	 * @param tokenString the token string
	 */
	public void enterModule(String tokenString);

	/**
	 * Exit module.
	 */
	public void exitModule();

	/**
	 * New net.
	 *
	 * @param netName the net name
	 * @param i the index of the net, -1 if a new one, otherwise the id if the referenced
	 * @param width the width
	 * @return the integer
	 */
	public Integer newNet(String netName, int i, int width) ;

	/**
	 * Gets the net width.
	 *
	 * @param intValue the int value
	 * @return the net width
	 */
	public int getNetWidth(int intValue);

	/**
	 * Append transition.
	 *
	 * @param signalId the int value
	 * @param currentTime the current time in ps
	 * @param decodedValues the decoded values
	 */
	public void appendTransition(int signalId, long currentTime, BitVector decodedValues);

}
