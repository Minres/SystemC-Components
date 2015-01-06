package com.minres.scviewer.database.vcd;

// TODO: Auto-generated Javadoc
/**
 * The Interface ITraceBuilder.
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
	 * @param intValue the int value
	 * @param fCurrentTime the f current time
	 * @param decodedValues the decoded values
	 */
	public void appendTransition(int intValue, long fCurrentTime, BitVector decodedValues);

}
