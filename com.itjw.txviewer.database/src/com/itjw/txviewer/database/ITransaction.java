package com.itjw.txviewer.database;

import java.util.List;
import java.util.Set;

public interface ITransaction {
	public Long getId();
	public ITrGenerator getGenerator();
	public EventTime getBeginTime();
	public EventTime getEndTime();
	public List<ITrAttribute> getBeginAttrs();
	public List<ITrAttribute> getEndAttrs();
	public List<ITrAttribute> getAttributes();
	public Set<ITransaction> getNextInRelationship(RelationType rel);
}
