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
package com.minres.scviewer.database.sqlite.db;

import java.lang.reflect.Field;


/**
 * An abstract class that handles insert/select-operations into/from a database
 * 
 * @author Tino for http://www.java-blog.com
 * 
 * @param <T>
 */
public abstract class AbstractDatabaseHandler<T> {

	/**
	 * The type of the objects that should be created and filled with values
	 * from the database or inserted into the database
	 */
	protected Class<T>     type;

	/**
	 * Contains the settings to create a connection to the database like
	 * host/port/database/user/password
	 */
	protected IDatabase     databaseConnectionFactory;

	/** The SQL-select-query */
	protected final String     query;

	/**
	 * Constructor
	 * 
	 * @param type
	 *            The type of the objects that should be created and filled with
	 *            values from the database or inserted into the database
	 * @param databaseConnecter
	 *            Contains the settings to create a connection to the database
	 *            like host/port/database/user/password
	 */
	protected AbstractDatabaseHandler(Class<T> type,
			IDatabase databaseConnectionFactory, String criterion) {

		this.databaseConnectionFactory = databaseConnectionFactory;
		this.type = type;
		this.query = createQuery(criterion);
	}

	/**
	 * Create the SQL-String to insert into / select from the database
	 * 
	 * @return the SQL-String
	 */
	protected abstract String createQuery(String criterion);

	/**
	 * 
	 * Creates a comma-separated-String with the names of the variables in this
	 * class
	 * 
	 * @param usePlaceHolders
	 *            true, if PreparedStatement-placeholders ('?') should be used
	 *            instead of the names of the variables
	 * @return
	 */
	protected String getColumns(boolean usePlaceHolders) {
		StringBuilder sb = new StringBuilder();

		boolean first = true;
		/* Iterate the column-names */
		for (Field f : type.getDeclaredFields()) {
			if (first)
				first = false;
			else
				sb.append(", ");

			if (usePlaceHolders)
				sb.append("?");
			else
				sb.append(f.getName());
		}

		return sb.toString();
	}
}
