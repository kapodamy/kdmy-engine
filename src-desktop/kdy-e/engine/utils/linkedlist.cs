using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Engine.Utils;

public class LinkedList<T> {

    private class LinkedListNode {
        public T item;
        public LinkedListNode next;
    }

    private LinkedListNode head;
    private LinkedListNode tail;
    private int count;

    public LinkedList() {

    }

    public void Destroy() {
        Destroy2(null);
    }

    public void Destroy2(Action<T> release_function = null) {
        LinkedListNode current = head;

        if (current != null) {
            if (release_function != null) {
                while (current != null) {
                    if (current.item != null) release_function(current.item);
                    LinkedListNode next = current.next;
                    //free(current);
                    current = next;
                }
            } else {
                while (current != null) {
                    LinkedListNode next = current.next;
                    //free(current);
                    current = next;
                }
            }
        }

        //free(*linkedlist);
        //*linkedlist = NULL;
        head = null;
        tail = null;
        count = 0;
    }

    public T GetFirstItem() {
        return head == null ? default(T) : head.item;
    }

    public T GetLastItem() {
        return this.tail != null ? this.tail.item : default(T);
    }

    public int AddItem(T obj) {
        LinkedListNode node = new LinkedListNode();

        node.item = obj;
        node.next = null;

        if (this.head == null) {
            this.head = node;
        } else {
            this.tail.next = node;
        }

        this.tail = node;
        this.count++;

        return this.count;
    }

    public bool HasItem(T obj) {
        LinkedListNode current = this.head;
        if (obj == null) {
            while (current != null) {
                if (current.item == null) return true;
                current = current.next;
            }
        } else {
            while (current != null) {
                if (obj.Equals(current.item)) return true;
                current = current.next;
            }
        }
        return false;
    }

    public T GetByIndex(int index) {
        if (index < 0 || index >= this.count) return default(T);

        LinkedListNode current = this.head;

        for (int i = 0 ; i < index && current != null ; i++) current = current.next;

        return current != null ? current.item : default(T);
    }

    public int RemoveItem(T obj) {
        int i = 0;
        LinkedListNode current = this.head;

        if (obj == null) {
            while (current != null) {
                if (current.item == null) {
                    this.RemoveItemAt(i);
                    return i;
                }
                current = current.next;
                i++;
            }
        } else {
            while (current != null) {
                if (obj.Equals(current.item)) {
                    this.RemoveItemAt(i);
                    return i;
                }
                current = current.next;
                i++;
            }
        }

        return -1;
    }

    public T RemoveItemAt(int index) {
        if (this.count < 1) return default(T);

        Debug.Assert(index >= 0 && index < this.count);

        LinkedListNode current;
        T item;

        if (this.count == 1) {
            item = this.head.item;
            //free(this.head);
            this.head = null;
            this.tail = null;
            this.count = 0;
            return item;
        }

        if (index == 0) {
            current = this.head.next;
            item = this.head.item;

            //free(this.head);

            this.head = current;
            this.count--;
            return item;
        }

        int i = 1;
        LinkedListNode previous = this.head;
        current = this.head.next;

        while (current != null) {
            if (index == i) {
                previous.next = current.next;

                if (this.tail == current)
                    this.tail = previous;

                this.count--;
                item = current.item;

                //free(current);
                return item;
            }

            i++;
            previous = current;
            current = current.next;
        }

        throw new InvalidOperationException("linkedlist is corrupted or in a invalid state");
    }

    public int Count() {
        return this.count;
    }

    public T[] ToArray() {
        if (this.count < 1) return null;

        int i = 0;
        T[] array = new T[this.count];
        LinkedListNode current = this.head;

        while (current != null) {
            array[i++] = current.item;
            current = current.next;
        }

        return array;
    }

    public T[] ToSolidArray(/*size_t item_size*/) {
        return ToArray();
    }

    public void Clear(Action<T> release_function = null) {
        LinkedListNode current = this.head;
        while (current != null) {
            if (release_function != null && current.item != null) release_function(current.item);
            LinkedListNode next = current.next;
            //free(current);
            current = next;
        }

        this.head = null;
        this.tail = null;
        this.count = 0;
    }

    public int IndexOf(T obj) {
        int i = 0;
        LinkedListNode current = this.head;

        if (obj == null) {
            while (current != null) {
                if (current.item == null) return i;
                current = current.next;
                i++;
            }
        } else {
            while (current != null) {
                if (obj.Equals(current.item)) return i;
                current = current.next;
                i++;
            }
        }

        return -1;
    }


    public void SetItem(int index, T item) {
        if (index < 0 || index >= this.count) throw new ArgumentOutOfRangeException("index");

        int i = 0;
        LinkedListNode current = this.head;
        while (current != null) {
            if (i == index) {
                current.item = item;
                return;
            }
            current = current.next;
            i++;
        }
    }


    public System.Collections.Generic.IEnumerator<T> GetEnumerator() {
        return new LinkedListEnumerator(this.head);
    }


    private struct LinkedListEnumerator : System.Collections.Generic.IEnumerator<T> {
        private readonly LinkedListNode head;
        private LinkedListNode current;
        private bool last;

        public LinkedListEnumerator(LinkedListNode head) {
            this.head = head;
            this.current = null;
            this.last = head == null;
        }


        public bool MoveNext() {
            if (last) return false;

            if (this.current == null) {
                this.current = this.head;
                this.last = this.head.next == null;
                return true;
            }

            this.current = this.current.next;
            if (this.current.next == null) this.last = true;

            return true;
        }

        public void Reset() {
            if (this.head == null) return;
            this.current = null;
            this.last = false;
        }

        void IDisposable.Dispose() {

        }


        object System.Collections.IEnumerator.Current => current.item;

        public T Current => current.item;
    }

}
