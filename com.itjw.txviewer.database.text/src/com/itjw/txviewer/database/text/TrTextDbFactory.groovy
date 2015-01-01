package com.itjw.txviewer.database.text

import com.itjw.txviewer.database.ITrDb
import com.itjw.txviewer.database.ITransactionDbFactory;

class TrTextDbFactory implements ITransactionDbFactory {

	@Override
	public ITrDb createDatabase() {
		return new TrTextDb();
	}

}
