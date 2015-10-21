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

import java.beans.IntrospectionException;
import java.beans.PropertyDescriptor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

/**
 * 
 * Class that creates a list of <T>s filled with values from the corresponding
 * database-table.
 * 
 * @author Tino for http://www.java-blog.com
 * 
 * @param <T>
 */
public class SQLiteDatabaseSelectHandler<T> extends AbstractDatabaseHandler<T> {
	
	public SQLiteDatabaseSelectHandler(Class<T> type,
			IDatabase databaseConnectionFactory) {
		super(type, databaseConnectionFactory, null);
	}

	public SQLiteDatabaseSelectHandler(Class<T> type,
			IDatabase databaseConnectionFactory, String criteria) {
		super(type, databaseConnectionFactory, criteria);
	}

	@Override
	protected String createQuery(String criterion) {

		StringBuilder sb = new StringBuilder();

		sb.append("SELECT ");
		sb.append(super.getColumns(false));
		sb.append(" FROM ");

		/* We assume the table-name exactly matches the simpleName of T */
		sb.append(type.getSimpleName());
		if(criterion!=null){
			sb.append(" WHERE ( ");
			sb.append(criterion);
			sb.append(" )");
		}
		return sb.toString();
	}

	/**
	 * Creates a list of <T>s filled with values from the corresponding
	 * database-table
	 * 
	 * @return List of <T>s filled with values from the corresponding
	 *         database-table
	 * 
	 * @throws SQLException
	 * @throws SecurityException
	 * @throws IllegalArgumentException
	 * @throws InstantiationException
	 * @throws IllegalAccessException
	 * @throws IntrospectionException
	 * @throws InvocationTargetException
	 */
	public synchronized List<T> selectObjects() throws SQLException,
			SecurityException, IllegalArgumentException,
			InstantiationException, IllegalAccessException,
			IntrospectionException, InvocationTargetException {

		Connection connection = null;
		Statement statement = null;
		ResultSet resultSet = null;

		try {
			connection = databaseConnectionFactory.createConnection();
			statement = connection.createStatement();
			resultSet = statement.executeQuery(query);

			return createObjects(resultSet);

		} finally {
			databaseConnectionFactory.close(resultSet, statement, connection);
		}
	}

	/**
	 * 
	 * Creates a list of <T>s filled with values from the provided ResultSet
	 * 
	 * @param resultSet
	 *            ResultSet that contains the result of the
	 *            database-select-query
	 * 
	 * @return List of <T>s filled with values from the provided ResultSet
	 * 
	 * @throws SecurityException
	 * @throws IllegalArgumentException
	 * @throws SQLException
	 * @throws InstantiationException
	 * @throws IllegalAccessException
	 * @throws IntrospectionException
	 * @throws InvocationTargetException
	 */
	private List<T> createObjects(ResultSet resultSet)
			throws SecurityException, IllegalArgumentException,
			SQLException, InstantiationException,
			IllegalAccessException, IntrospectionException,
			InvocationTargetException {

		List<T> list = new ArrayList<T>();

		while (resultSet.next()) {

			T instance = type.newInstance();

			for (Field field : type.getDeclaredFields()) {

				/* We assume the table-column-names exactly match the variable-names of T */
				Object value = resultSet.getObject(field.getName());
				if(value!=null){
					PropertyDescriptor propertyDescriptor = new PropertyDescriptor(field.getName(), type);
					propertyDescriptor.getWriteMethod().invoke(instance, value);
				}
			}

			list.add(instance);
		}
		return list;
	}
}
