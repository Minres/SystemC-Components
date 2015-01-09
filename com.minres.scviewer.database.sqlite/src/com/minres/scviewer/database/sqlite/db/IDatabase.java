package com.minres.scviewer.database.sqlite.db;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

/**
 *
 * Creates a connection to a database.
 *
 * @author Tino
 * @created 03.12.2008
 *
 */
public interface IDatabase {

	/**
	 * Establishes a new connection to the database
	 *
	 * @return A new connection to the database
	 * @throws SQLException
	 */
	public Connection createConnection() throws SQLException;

	/**
	 * Returns the connection url
	 *
	 * @return
	 */
	public String getConnectionUrl();

	/**
	 * releases the result set
	 *
	 * @return
	 */
	public void close(ResultSet resultSet, Statement statement, Connection connection);

	/**
	 * releases the preparedStatement
	 *
	 * @return
	 */
	public void close(PreparedStatement preparedStatement, Connection connection);

	public void setData(String name, Object value);
	
	public Object getData(String name); 
} 