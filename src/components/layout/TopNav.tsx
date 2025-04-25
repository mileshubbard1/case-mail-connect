
import { Link } from "react-router-dom";
import { cn } from "@/lib/utils";
import { Button } from "@/components/ui/button";
import { Inbox, FileText, Search, Users, Calendar } from "lucide-react";

interface NavLinkProps {
  to: string;
  icon: React.ComponentType<React.SVGProps<SVGSVGElement>>;
  label: string;
  isActive?: boolean;
}

const NavLink = ({ to, icon: Icon, label, isActive }: NavLinkProps) => (
  <Link to={to}>
    <Button
      variant="ghost"
      className={cn(
        "flex items-center gap-2",
        isActive ? "bg-legal-light text-legal-navy font-medium" : "hover:bg-legal-light/50"
      )}
    >
      <Icon className="h-5 w-5" />
      <span>{label}</span>
    </Button>
  </Link>
);

export function TopNav() {
  return (
    <div className="border-b bg-white">
      <div className="px-6 py-2 flex items-center gap-2">
        <NavLink to="/" icon={Inbox} label="Inbox" isActive />
        <NavLink to="/cases" icon={FileText} label="Cases" />
        <NavLink to="/search" icon={Search} label="Search" />
        <NavLink to="/clients" icon={Users} label="Clients" />
        <NavLink to="/calendar" icon={Calendar} label="Calendar" />
      </div>
    </div>
  );
}
