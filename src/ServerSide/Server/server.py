from ctypes import *
from http.server import BaseHTTPRequestHandler, HTTPServer

libc = CDLL("server.so")

libc.LogMessage.argtypes = [c_char_p]

libc.MakeShop.restype = c_void_p

libc.DeleteShop.argtypes = [c_void_p]

libc.AddItemToShop.argtypes = [c_void_p, c_ulonglong]

libc.DeleteItemFromShop.argtypes = [c_void_p, c_ulonglong]

libc.ShopOrderIsEmpty.argtypes = [c_void_p]
libc.ShopOrderIsEmpty.restype = c_int

libc.BuyOrderToString.argtypes = [c_void_p]
libc.BuyOrderToString.restype = c_char_p

libc.DBUpdateItem.argtypes = [c_ulonglong, c_int]

libc.DBUpdateBox.argtypes = [c_ulonglong, c_int]

libc.DBGetItems.restype = c_char_p

libc.DBGetBoxes.restype = c_char_p

libc.DBGetOrders.restype = c_char_p


print('Start')
libc.ConfigureLogger()
libc.OpenDataBase()
print('Opened db')
shop = libc.MakeShop()
print('made shop')

print('Ready!')


class HandleRequests(BaseHTTPRequestHandler):

    def _set_headers(self):
        self.send_response(200)
        self.send_header('Content-type', 'plain/text')
        self.end_headers()

    def do_GET(self):
        path = str(self.path)
        args = path.split('/')[1:]
        qtype = args[0]

        global shop

        content = 'ok'
    
        if qtype == 'add_item':
            itemID = int(args[1])
            libc.AddItemToShop(shop, itemID)
        elif qtype == 'delete_item':
            itemID = int(args[1])
            libc.DeleteItemFromShop(shop, itemID)
        elif qtype == 'order_is_empty':
            orderIsEmpty = libc.ShopOrderIsEmpty(shop)
            content = str(orderIsEmpty)
        elif qtype == 'buy':
            content = libc.BuyOrderToString(shop).decode('utf-8')
        elif qtype == 'update_item':
            itemID = int(args[1])
            amount = int(args[2])
            libc.DBUpdateItem(itemID, amount)
        elif qtype == 'update_box':
            boxID = int(args[1])
            amount = int(args[2])
            libc.DBUpdateBox(boxID, amount)
        elif qtype == 'get_items':
            content = libc.DBGetItems().decode('utf-8')
        elif qtype == 'get_boxes':
            content = libc.DBGetBoxes().decode('utf-8')
        elif qtype == 'save_order':
            libc.DBSaveOrder(path.encode('utf-8'))
        elif qtype == 'get_orders':
            content = libc.DBGetOrders().decode('utf-8')
        else:
            libc.LogMessage('Got undefined query: {0}'.format(qtype).encode('utf-8'))
            content = 'undefined query'
        self._set_headers()
        self.wfile.write(content.encode('utf-8')) 

HTTPServer(('localhost', 8080), HandleRequests).serve_forever()

# usage: http://localhost:8080/get_orders

